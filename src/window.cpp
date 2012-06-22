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

#define USE_SSE 1

#if USE_SSE
#include <emmintrin.h>
#endif

//#import  "interface.tlb" raw_interfaces_only
#include "interface.tlh"
#include "util.h"

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Shell32.lib")


namespace Ck {

_COM_SMARTPTR_TYPEDEF(IDropTargetHelper, __uuidof(IDropTargetHelper));

#define WM_USER_DROP_HDROP  WM_USER+10
#define WM_USER_DROP_WSTR   WM_USER+11

class droptarget: public IUnknownImpl2<IDropTarget>{
protected:
    HWND   m_parent;
    DWORD  m_eff;
    DWORD  m_key;
    IDropTargetHelperPtr m_help;
public:
    droptarget(HWND hwnd):m_parent(hwnd),m_eff(DROPEFFECT_NONE), m_key(0) {
        m_help.CreateInstance(CLSID_DragDropHelper);
    }
    STDMETHOD(DragOver)(DWORD key, POINTL pt, DWORD* eff) {
        *eff = m_eff;
        m_key = key;
        if (m_help) m_help->DragOver((POINT*)&pt, *eff);
        return S_OK;
    }
    STDMETHOD(DragLeave)(void) {
        if (m_help) m_help->DragLeave();
        return S_OK;
    }
    STDMETHOD(DragEnter)(IDataObject* data, DWORD key, POINTL pt, DWORD* eff) {
        FORMATETC fmt = { CF_HDROP, 0,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};
        HRESULT hr = data->QueryGetData(&fmt);
        if (FAILED(hr)) {
            fmt.cfFormat = CF_UNICODETEXT;
            hr = data->QueryGetData(&fmt);
        }
        if (FAILED(hr))
            *eff = DROPEFFECT_NONE;
        m_eff = *eff;
        m_key = key;
        if (m_help) m_help->DragEnter(m_parent, data, (POINT*)&pt, *eff);
        return S_OK;
    }
    STDMETHOD(Drop)(IDataObject* data, DWORD key, POINTL pt, DWORD* eff) {
        *eff = m_eff;
        if (m_help) m_help->Drop(data, (POINT*)&pt, *eff);
        if (m_eff != DROPEFFECT_NONE) {
            STGMEDIUM stg;
            memset(&stg,0,sizeof(stg));
            FORMATETC fmt = { CF_HDROP, 0,DVASPECT_CONTENT,-1,TYMED_HGLOBAL};
            if (SUCCEEDED(data->GetData(&fmt, &stg))) {
                SendMessage(m_parent, WM_USER_DROP_HDROP, (WPARAM)stg.hGlobal, (LPARAM)m_key);
                ReleaseStgMedium(&stg);
            }
            else {
                fmt.cfFormat = CF_UNICODETEXT;
                if (SUCCEEDED(data->GetData(&fmt, &stg))) {
                    SendMessage(m_parent, WM_USER_DROP_WSTR, (WPARAM)stg.hGlobal, (LPARAM)m_key);
                    ReleaseStgMedium(&stg);
                }
            }
        }
        return S_OK;
    }
};




class Bmp32{
protected:
    int    m_width;
    int    m_height;
    DWORD*    m_pixels;
public:
    int    width() { return m_width; }
    int    height() { return m_height; }
    DWORD*    pixels() { return m_pixels; }
    Bmp32(): m_width(0),m_height(0),m_pixels(0) {}
};

class BgBmp: public Bmp32{
private:
    ULONG  m_ref;
    BSTR   m_path;
protected:
    void _finalize() {
        if (m_path) SysFreeString(m_path);
        if (m_pixels) delete [] m_pixels;
    }
    ~BgBmp() {
        trace("bgbmp::dtor\n");
        _finalize();
    }
public:
    BSTR   path() { return m_path; }
    ULONG  AddRef() { return ++m_ref; }
    ULONG  Release() { ULONG n=--m_ref; if (!n) delete this; return n; }
    BgBmp(BSTR cygpath): m_ref(0),m_path(0) {
        Gdiplus::Bitmap* bmp = 0;
        BSTR winpath = 0;
        try{
            m_path = SysAllocStringLen(cygpath, SysStringLen(cygpath));
            winpath = Util::to_windows_path(cygpath);

            bmp = new Gdiplus::Bitmap(winpath, FALSE);
            UINT w = bmp->GetWidth();
            UINT h = bmp->GetHeight();
            if (w<1 || 8192<w || h<1 || 8192<h)
                throw (HRESULT)E_FAIL;
            m_pixels = new DWORD[w * h];
            m_width  = w;
            m_height = h;
            Gdiplus::Rect  rect(0,0,w,h);
            Gdiplus::BitmapData data;
            data.Width  = w;
            data.Height = h;
            data.Stride = w * sizeof(DWORD);
            data.PixelFormat = PixelFormat32bppARGB;
            data.Scan0 = m_pixels;
            if (Gdiplus::Ok != bmp->LockBits(&rect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf, data.PixelFormat, &data))
                throw (HRESULT)E_FAIL;
            bmp->UnlockBits(&data);
        }
        catch(...) {
            if (winpath) SysFreeString(winpath);
            if (bmp) delete bmp;
            _finalize();
            throw;
        }
        if (winpath) SysFreeString(winpath);
        if (bmp) delete bmp;
        trace("bgbmp::ctor\n");
    }
};

class BgBmpMgr{
private:
    static const int MAXSIZE = 64;
    int    m_count;
    BgBmp*    m_array [MAXSIZE];
protected:
    void clear() {
        for (int i=0; i < m_count; i++)
            m_array[i]->Release();
        m_count=0;
    }
    void add(BgBmp* p) {
        m_array[m_count++] = p;
        p->AddRef();
    }
    void remove(BgBmp* p) {
        for (int i=0; i < m_count; i++) {
            if (m_array[i]==p) {
                p->Release();
                memmove(m_array+i, m_array+i+1, sizeof(BgBmp*) * (m_count-i-1));
                m_count--;
                break;
            }
        }
    }
public:
    ~BgBmpMgr() { clear(); }
    BgBmpMgr(): m_count(0) { memset(m_array,0,sizeof(m_array)); }
    //
    BgBmp* get(BSTR cygpath) {
        if (cygpath) {
            BgBmp* p;
            for (int i=0; i<m_count; i++) {
                p = m_array[i];
                if (_wcsicmp(p->path(), cygpath)==0) {
                    p->AddRef();
                    return p;
                }
            }
            if (m_count < MAXSIZE) {
                try{
                    p = new BgBmp(cygpath);
                    p->AddRef();
                    add(p);
                    return p;
                }
                catch(...) {
                }
            }
        }
        return 0;
    }
    void release(BgBmp* p) {
        if (p->Release() <= 1)
            remove(p);
    }
};
static BgBmpMgr g_bgbmp_manager;



#if USE_SSE
static const __m128i  _cc_all256_epi16     = {0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1};                //01000100 01000100 01000100 01000100
static const __m128i  _cc_all128_epi16     = {-128,0,-128,0,-128,0,-128,0, -128,0,-128,0,-128,0,-128,0};    //00800080 00800080 00800080 00800080
static const __m128i  _cc_alpha_mask_epi16 = {0,0,0,0,0,0,-1,0, 0,0,0,0,0,0,-1,0};                //00FF0000 00000000 00FF0000 00000000

inline __m128i  _cc_div255_epi16(__m128i a) {
    a = _mm_add_epi16(a, _cc_all128_epi16);
    return _mm_srli_epi16(_mm_add_epi16(a,_mm_srli_epi16(a,8)), 8);
}

inline UINT  _cc_div255(UINT n) {
    n+=128;
    return ((n>>8)+n)>>8;
}
#endif

inline DWORD  composite_argb(DWORD dst, const DWORD src) {
    #if USE_SSE
    const __m128i  ZERO = _mm_setzero_si128();
    const __m128i  ARGB = _mm_unpacklo_epi8(_mm_cvtsi32_si128(src), ZERO);
    const UINT  sa = src>>24;
    UINT  da,sf;

    da = ((BYTE*)&dst)[3];
    da = _cc_div255(da * (255-sa)) + sa;
    sf = (da) ? ((sa<<8)/da) : 0;
    ((BYTE*)&dst)[3] = da;

    __m128i  f0, f1, color;
    f0 = _mm_shufflelo_epi16(_mm_cvtsi32_si128(sf), _MM_SHUFFLE(3,0,0,0));    //00000000 00000000 000000s0 00s000s0
    f1 = _mm_sub_epi16(_cc_all256_epi16, f0);

    color = _mm_unpacklo_epi8(_mm_cvtsi32_si128(dst), ZERO);        //00000000 00000000 00AA00RR 00GG00BB

    color = _mm_add_epi16(_mm_mullo_epi16(color, f1), _mm_mullo_epi16(ARGB, f0));// ( (D*(256-sa)) + (S*sa) ) / 256
    color = _mm_srli_epi16(color, 8);

    color = _mm_packus_epi16(color,color);                    //00000000 00000000 00000000 AARRGGBB
    return _mm_cvtsi128_si32(color);
    #else
    UINT  da, sa, sf;

    da = dst>>24;
    sa = src>>24;

    da = (da * (255-sa)) / 255;
    da = da + sa;
    sf = (da) ? ((sa<<16)/da) : 0;

    DWORD  rv;
    BYTE*  O = (BYTE*)(&rv);
    const BYTE* D = (const BYTE*)(&dst);
    const BYTE* S = (const BYTE*)(&src);
    O[0] = (( (S[0]-D[0]) * sf) >> 16) + D[0];
    O[1] = (( (S[1]-D[1]) * sf) >> 16) + D[1];
    O[2] = (( (S[2]-D[2]) * sf) >> 16) + D[2];
    O[3] = da;
    return rv;
    #endif
}

inline DWORD  composite_argb_inv(DWORD dst, const DWORD src) {
    return composite_argb(dst,src) ^ 0xFF000000;
}

inline DWORD  to_pargb(const DWORD argb) {
    #if USE_SSE
    const __m128i  ZERO = _mm_setzero_si128();
    __m128i color, alpha;

    color = _mm_unpacklo_epi8(_mm_cvtsi32_si128(argb), ZERO);    //00000000 00000000 00aa00rr 00gg00bb

    alpha = _mm_shufflelo_epi16(color, _MM_SHUFFLE(3,3,3,3));    //00000000 00000000 00aa00aa 00aa00aa
    alpha = _mm_or_si128(alpha, _cc_alpha_mask_epi16);        //00FF0000 00000000 00FF00aa 00aa00aa

    color = _mm_mullo_epi16(color,alpha);
    color = _cc_div255_epi16(color);

    color = _mm_packus_epi16(color,color);
    return _mm_cvtsi128_si32(color);
    #else
    DWORD  rv;
    BYTE*  O = (BYTE*)(&rv);
    const BYTE*  S = (const BYTE*)(&argb);
    const UINT   a = S[3];
    O[0] = (S[0] * a) / 255;
    O[1] = (S[1] * a) / 255;
    O[2] = (S[2] * a) / 255;
    O[3] = a;
    return rv;
    #endif
}






class OsBmp2: public Bmp32{
private:
    HBITMAP m_handle;
    BgBmp*    m_bg;
    int*    m_bg_x;
    int*    m_bg_y;
    DWORD    m_bg_place;
    //
    void _clear_bgbmp() {
        if (m_bg) {
            g_bgbmp_manager.release(m_bg);
            m_bg = 0;
        }
    }
    void _clear_index() {
        if (m_bg_x) {
            delete [] m_bg_x;
            m_bg_x = m_bg_y = 0;
        }
    }
    void _clear_hbmp() {
        if (m_handle) {
            DeleteObject(m_handle);
            m_width = m_height = 0;
            m_pixels = 0;
            m_handle = 0;
        }
    }
    void _build_index(int* dst, int max, int max2, int bgmax, int bgmax2, int repeat, int align) {
        double scale,offset;
        int i,n;
        switch(repeat) {
        case Place_NoRepeat:
            switch(align) {
            case Align_Far:        n = -(max - bgmax);break;
            case Align_Center:    n = -((max - bgmax)/2);break;
            case Align_Near: default: n = 0;break;
            }
            for (i=0; i < max; i++,n++)
                dst[i] = (n<0)? 0: (n<bgmax)? n: bgmax-1;
            break;
        case Place_Repeat:
            switch(align) {
            case Align_Far:        n = bgmax - (max % bgmax);break;
            case Align_Center:    n = bgmax - (((max-bgmax)/2) % bgmax);break;
            case Align_Near: default: n = bgmax - 0; break;
            }
            for (int i=0; i < max; i++,n++)
                dst[i] = n % bgmax;
            break;
        case Place_Zoom://maintain aspect ratio
            scale = (max*bgmax2/(double)bgmax > max2) ? (bgmax2/(double)max2) : (bgmax/(double)max);
            switch(align) {
            case Align_Far:        offset = -(max - (bgmax/scale));break;
            case Align_Center:    offset = -((max - (bgmax/scale))/2.0);break;
            case Align_Near: default: offset = 0.0;break;
            }
            for (int i=0; i < max; i++) {
                n = int((i+offset) * scale);
                dst[i] = (n<0)? 0: (n<bgmax)? n: bgmax-1;
            }
            break;
        case Place_Scale:
        default:
            scale = bgmax / double(max);
            for (int i=0; i < max; i++)
                dst[i] = int(i * scale);
            break;
        }
    }
    void _build_index() {
        if (m_bg) {
            _clear_index();
            m_bg_x = new int[m_width + m_height];
            m_bg_y = m_bg_x + m_width;
            _build_index(m_bg_x, m_width, m_height, m_bg->width(), m_bg->height(), (m_bg_place>>8 )&0xff, (m_bg_place>>0 )&0xff);
            _build_index(m_bg_y, m_height, m_width, m_bg->height(), m_bg->width(), (m_bg_place>>24)&0xff, (m_bg_place>>16)&0xff);
        }
    }
public:
    ~OsBmp2() { _clear_hbmp(); _clear_index(); _clear_bgbmp(); }
    OsBmp2()
        : m_handle(0),
          m_bg(0),
          m_bg_x(0),
          m_bg_y(0),
          m_bg_place(0x02010201) { //  31-24=repeatY 23-16=alignY 15-8=repeatX 7-0=alignX
    }
    HBITMAP  handle() { return m_handle; }
    bool     UseBgBmp() { return (m_bg_x) ? true : false; }
    DWORD    GetBgPlace() { return m_bg_place; }
    BSTR     GetBgFile() { return m_bg ? m_bg->path() : 0; }
    void SetBgPlace(DWORD n) {
        m_bg_place = n;
        _build_index();
    }
    void SetBgFile(BSTR cygpath) {
        BgBmp* n = g_bgbmp_manager.get(cygpath);
        _clear_bgbmp();
        _clear_index();
        m_bg = n;
        _build_index();
    }
    void Resize(int w, int h) {
        _clear_hbmp();
        _clear_index();
        BITMAPINFOHEADER bh={sizeof(bh), w, -h, 1, 32, BI_RGB, 0, 0,0, 0,0};
        void* px;
        m_handle = CreateDIBSection(0, (BITMAPINFO*)&bh, DIB_RGB_COLORS, &px, 0,0);
        if (m_handle) {
            m_width = w;
            m_height = h;
            m_pixels = (DWORD*)px;
            _build_index();
        }
    }

    void Fill(const DWORD argb) {
        const DWORD pargb = to_pargb(argb);
        const RECT  rc = {0,0,m_width,m_height};
        if (m_bg_x == 0) {
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int y=rc.top; y < rc.bottom; y++) {
                for (int x=rc.left; x < rc.right; x++)
                    dst_ptr[x] = pargb;
                dst_ptr += m_width;
            }
        }
        else {
            #if USE_SSE
            const __m128i  ZERO  = _mm_setzero_si128();
            const __m128i  PARGB = _mm_unpacklo_epi8( _mm_shuffle_epi32( _mm_cvtsi32_si128(pargb), _MM_SHUFFLE(0,0,0,0)), ZERO);
            const UINT     invsa = 255-(pargb>>24);
            __m128i        alpha, color;

            const bool  rcw_odd = (rc.right - rc.left) & 1;
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int y=rc.top; y < rc.bottom; y++) {
                DWORD* bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[y]);
                int x = rc.left;
                for (; x < rc.right; x+=2) {
                    __m128i  WORK;
                    ((DWORD*)&WORK)[0] = bg_ptr[m_bg_x[x+0]];
                    ((DWORD*)&WORK)[1] = bg_ptr[m_bg_x[x+1]];

                    ((BYTE*)&WORK)[3] = _cc_div255( ((BYTE*)&WORK)[3] * invsa );
                    ((BYTE*)&WORK)[7] = _cc_div255( ((BYTE*)&WORK)[7] * invsa );

                    color = _mm_unpacklo_epi8(_mm_loadl_epi64(&WORK), ZERO);    //00aa00rr 00gg00bb 00AA00RR 00GG00BB
                    alpha = _mm_shufflelo_epi16(color, _MM_SHUFFLE(3,3,3,3));
                    alpha = _mm_shufflehi_epi16(alpha, _MM_SHUFFLE(3,3,3,3));    //00aa00aa 00aa00aa 00AA00AA 00AA00AA
                    alpha = _mm_or_si128(alpha, _cc_alpha_mask_epi16);        //00FF00aa 00aa00aa 00FF00AA 00AA00AA

                    color = _mm_mullo_epi16(color, alpha);// c=(c*a)/255+s;
                    color = _cc_div255_epi16(color);
                    color = _mm_add_epi16(color, PARGB);

                    color = _mm_packus_epi16(color,color);
                    _mm_storel_epi64((__m128i*)(dst_ptr+x), color);
                }
                if (rcw_odd) {
                    DWORD  WORK = bg_ptr[m_bg_x[x+0]];

                    ((BYTE*)&WORK)[3] = _cc_div255( ((BYTE*)&WORK)[3] * invsa );

                    color = _mm_unpacklo_epi8(_mm_cvtsi32_si128(WORK), ZERO);    //00000000 00000000 00AA00RR 00GG00BB
                    alpha = _mm_shufflelo_epi16(color, _MM_SHUFFLE(3,3,3,3));    //00000000 00000000 00AA00AA 00AA00AA
                    alpha = _mm_or_si128(alpha, _cc_alpha_mask_epi16);        //00FF0000 00000000 00FF00AA 00AA00AA

                    color = _mm_mullo_epi16(color, alpha);// c=(c*a)/255+s;
                    color = _cc_div255_epi16(color);
                    color = _mm_add_epi16(color, PARGB);

                    color = _mm_packus_epi16(color,color);
                    dst_ptr[x] = _mm_cvtsi128_si32(color);
                }
                dst_ptr += m_width;
            }
            #else
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int y=rc.top; y < rc.bottom; y++) {
                DWORD* bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[y]);
                int x = rc.left;
                for (; x < rc.right; ++x) {
                    const BYTE* S = (const BYTE*)&(pargb);
                    const BYTE* D = (const BYTE*)&(bg_ptr[m_bg_x[x+0]]);
                    BYTE* O = (BYTE*)(dst_ptr+x);
                    UINT sa = S[3];
                    UINT da = D[3];

                    da = (da * (255-sa)) / 255;
                    O[0] = (D[0] * da) / 255 + S[0];
                    O[1] = (D[1] * da) / 255 + S[1];
                    O[2] = (D[2] * da) / 255 + S[2];
                    O[3] = da + sa;
                }
                dst_ptr += m_width;
            }
            #endif
        }
    }

    void Text_fill(const RECT& rc, DWORD argb) {
        if (m_bg_x == 0) {
            argb = argb ^ 0xFF000000 | 0x01000000;
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int y=rc.top; y < rc.bottom; y++) {
                for (int x=rc.left; x < rc.right; x++)
                    dst_ptr[x] = argb;
                dst_ptr += m_width;
            }
        }
        else {
            #if USE_SSE
            const __m128i  ZERO  = _mm_setzero_si128();
            const __m128i  ARGB  = _mm_unpacklo_epi8(_mm_set1_epi32(argb), ZERO);
            const UINT     sa = argb>>24;
            const int      rcw_odd = (rc.right - rc.left) & 1;

            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int y=rc.top; y < rc.bottom; y++) {
                DWORD* bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[y]);
                int x = rc.left;
                for (; x < rc.right-1; x+=2) {
                    __m128i  color,FS,FD,WORK;
                    UINT     da, sf0,sf1;

                    ((DWORD*)&WORK)[0] = bg_ptr[m_bg_x[x+0]];
                    ((DWORD*)&WORK)[1] = bg_ptr[m_bg_x[x+1]];

                    da = ((BYTE*)&WORK)[3];
                    da = _cc_div255(da * (255-sa)) + sa;
                    sf0 = (da) ? ((sa<<8)/da) : 0;
                    ((BYTE*)&WORK)[3] = ~da | 1;

                    da = ((BYTE*)&WORK)[7];
                    da = _cc_div255(da * (255-sa)) + sa;
                    sf1 = (da) ? ((sa<<8)/da) : 0;
                    ((BYTE*)&WORK)[7] = ~da | 1;

                    FS = _mm_shufflelo_epi16(_mm_cvtsi32_si128(sf0), _MM_SHUFFLE(3,0,0,0));    //00000000 00000000 000000s0 00s000s0
                    FD = _mm_shufflelo_epi16(_mm_cvtsi32_si128(sf1), _MM_SHUFFLE(3,0,0,0));    //00000000 00000000 000000s1 00s100s1
                    FS = _mm_unpacklo_epi64(FS,FD);                        //000000s1 00s100s1 000000s0 00s000s0
                    FD = _mm_sub_epi16(_cc_all256_epi16, FS);

                    color = _mm_unpacklo_epi8(_mm_loadl_epi64(&WORK), ZERO);        //00aa00rr 00gg00bb 00AA00RR 00GG00BB

                    color = _mm_add_epi16(_mm_mullo_epi16(color, FD), _mm_mullo_epi16(ARGB,  FS));//((color*(256-sf)) + (argb*sf)) / 256;
                    color = _mm_srli_epi16(color, 8);

                    color = _mm_packus_epi16(color,color);                    //00000000 00000000 aarrggbb AARRGGBB
                    _mm_storel_epi64((__m128i*)(dst_ptr+x), color);
                }
                if (rcw_odd) {
                    __m128i  color,FS,FD;
                    UINT     da, sf0;

                    DWORD  WORK = bg_ptr[m_bg_x[x+0]];

                    da = ((BYTE*)&WORK)[3];
                    da = _cc_div255(da * (255-sa)) + sa;
                    sf0 = (da) ? ((sa<<8)/da) : 0;
                    ((BYTE*)&WORK)[3] = ~da | 1;

                    FS = _mm_shufflelo_epi16(_mm_cvtsi32_si128(sf0), _MM_SHUFFLE(3,0,0,0));    //00000000 00000000 000000s0 00s000s0
                    FD = _mm_sub_epi16(_cc_all256_epi16, FS);

                    color = _mm_unpacklo_epi8(_mm_cvtsi32_si128(WORK), ZERO);        //00000000 00000000 00AA00RR 00GG00BB

                    color = _mm_add_epi16(_mm_mullo_epi16(color, FD), _mm_mullo_epi16(ARGB,  FS));//((color*(256-sf)) + (argb*sf)) / 256;
                    color = _mm_srli_epi16(color, 8);

                    color = _mm_packus_epi16(color,color);                    //00000000 00000000 00000000 AARRGGBB
                    dst_ptr[x] = _mm_cvtsi128_si32(color);
                }
                dst_ptr += m_width;
            }
            #else
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int y=rc.top; y < rc.bottom; y++) {
                DWORD* bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[y]);
                int x = rc.left;
                for (; x < rc.right; ++x) {
                    const BYTE* S = (const BYTE*)&(argb);
                    const BYTE* D = (const BYTE*)&(bg_ptr[m_bg_x[x+0]]);
                    BYTE* O = (BYTE*)(dst_ptr+x);
                    UINT  sa,da,sf;

                    sa = S[3];
                    da = D[3];
                    da = (da * (255-sa)) / 255;
                    da = da + sa;
                    sf = (da) ? ((sa<<16)/da) : 0;

                    O[0] = (((S[0] - D[0]) * sf) >> 16) + D[0];// ((D[0] * (1-sf)) + (S[0] * sf))
                    O[1] = (((S[1] - D[1]) * sf) >> 16) + D[1];
                    O[2] = (((S[2] - D[2]) * sf) >> 16) + D[2];
                    O[3] = ~da | 1;
                }
                dst_ptr += m_width;
            }
            #endif
        }
    }
    void Text_rect(const RECT& rc, DWORD argb) {
        if (m_bg_x == 0) {
            argb = argb ^ 0xFF000000 | 0x01000000;
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            for (int x=rc.left; x < rc.right; x++)
                dst_ptr[x] = argb;
            dst_ptr += m_width;
            for (int y=rc.top+1; y < rc.bottom-1; y++) {
                dst_ptr[rc.left   ] = argb;
                dst_ptr[rc.right-1] = argb;
                dst_ptr += m_width;
            }
            for (int x=rc.left; x < rc.right; x++)
                dst_ptr[x] = argb;
        }
        else {
            DWORD* dst_ptr = m_pixels + (m_width * rc.top);
            DWORD* bg_ptr;
            {
                bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[rc.top]);
                for (int x=rc.left; x < rc.right; ++x) {
                    dst_ptr[x] = composite_argb_inv( bg_ptr[m_bg_x[x+0]], argb );
                }
                dst_ptr += m_width;
            }
            for (int y=rc.top+1; y < rc.bottom-1; ++y) {
                bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[y]);
                dst_ptr[rc.left   ] = composite_argb_inv( bg_ptr[m_bg_x[rc.left   ]], argb );
                dst_ptr[rc.right-1] = composite_argb_inv( bg_ptr[m_bg_x[rc.right-1]], argb );
                dst_ptr += m_width;
            }
            {
                bg_ptr = m_bg->pixels() + (m_bg->width() * m_bg_y[rc.bottom-1]);
                for (int x=rc.left; x < rc.right; ++x) {
                    dst_ptr[x] = composite_argb_inv( bg_ptr[m_bg_x[x+0]], argb );
                }
            }
        }
    }

    void Text_glow(const RECT& rc, const BYTE alpha) {
        #define AddAlpha(FL,PX) \
            if (!(FL & 0x01000000)) { \
                BYTE* p = (BYTE*)(&PX); \
                if (p[3] & 1) { \
                    p[3] = (p[3] > alpha) ? ((p[3]-alpha)|1) : (1); \
                } \
            }

        const int    rcw = rc.right - rc.left;
        DWORD  *p0,*p1,*p2;
        DWORD  a0,a1,a2;
        int    x, y = rc.top;

        p1 = m_pixels + (m_width * y) + rc.left;
        p2 = p1 + m_width;
        a0 = p1[0] & p2[0];
        a1 = p1[1] & p2[1];
        AddAlpha(a0&a1, p1[0]);
        for (x=2; x < rcw; x++) {
            a2 = p1[x] & p2[x];
            AddAlpha(a0&a1&a2, p1[x-1]);
            a0=a1;a1=a2;
        }
        AddAlpha(a0&a1, p1[x-1]);

        //---
        for (y=rc.top; y < rc.bottom-2; y++) {
            p0 = m_pixels + (m_width * y) + rc.left;
            p1 = p0 + m_width;
            p2 = p0 + m_width*2;
            a0 = p0[0] & p1[0] & p2[0];
            a1 = p0[1] & p1[1] & p2[1];
            AddAlpha(a0&a1, p1[0]);
            for (x=2; x < rcw; x++) {
                a2 = p0[x] & p1[x] & p2[x];
                AddAlpha(a0&a1&a2, p1[x-1]);
                a0=a1;a1=a2;
            }
            AddAlpha(a0&a1, p1[x-1]);
        }
        //---

        p0 = m_pixels + (m_width * y) + rc.left;
        p1 = p0 + m_width;
        a0 = p0[0] & p1[0];
        a1 = p0[1] & p1[1];
        AddAlpha(a0&a1, p1[0]);
        for (x=2; x < rcw; x++) {
            a2 = p0[x] & p1[x];
            AddAlpha(a0&a1&a2, p1[x-1]);
            a0=a1;a1=a2;
        }
        AddAlpha(a0&a1, p1[x-1]);
        #undef AddAlpha
    }

    void Text_to_pargb(const RECT& rc) {
        #if USE_SSE
        const __m128i  ZERO = _mm_setzero_si128();
        const int  rcw_odd = (rc.right - rc.left) & 1;
        DWORD* dst_ptr = m_pixels + (m_width * rc.top);
        for (int y=rc.top; y < rc.bottom; ++y) {
            int x = rc.left;
            for (; x < rc.right-1; x+=2) {
                __m128i color,alpha;
                color = _mm_loadl_epi64((__m128i*)(dst_ptr+x));        //00000000 00000000 aarrggbb aarrggbb
                color = _mm_unpacklo_epi8(color, ZERO);            //00aa00rr 00gg00bb 00aa00rr 00gg00bb
                color = _mm_xor_si128(color, _cc_alpha_mask_epi16);    //alpha invert

                alpha = _mm_shufflelo_epi16(color, _MM_SHUFFLE(3,3,3,3));
                alpha = _mm_shufflehi_epi16(alpha, _MM_SHUFFLE(3,3,3,3));
                alpha = _mm_or_si128(alpha, _cc_alpha_mask_epi16);    //00FF00AA 00AA00AA 00FF00AA 00AA00AA

                color = _mm_mullo_epi16(color,alpha);
                color = _cc_div255_epi16(color);

                color = _mm_packus_epi16(color,color);
                _mm_storel_epi64((__m128i*)(dst_ptr+x), color);
            }
            if (rcw_odd) {
                __m128i color,alpha;
                color = _mm_cvtsi32_si128(dst_ptr[x]);
                color = _mm_unpacklo_epi8(color, ZERO);            //00000000 00000000 00aa00rr 00gg00bb
                color = _mm_xor_si128(color, _cc_alpha_mask_epi16);    //alpha invert

                alpha = _mm_shufflelo_epi16(color, _MM_SHUFFLE(3,3,3,3));
                alpha = _mm_or_si128(alpha, _cc_alpha_mask_epi16);    //00FF0000 00000000 00FF00AA 00AA00AA

                color = _mm_mullo_epi16(color,alpha);
                color = _cc_div255_epi16(color);

                color = _mm_packus_epi16(color,color);
                dst_ptr[x] = _mm_cvtsi128_si32(color);
            }
            dst_ptr += m_width;
        }
        #else
        DWORD* dst_ptr = m_pixels + (m_width * rc.top);
        for (int y=rc.top; y < rc.bottom; ++y) {
            int x = rc.left;
            for (; x < rc.right; ++x) {
                dst_ptr[x] = to_pargb(dst_ptr[x] ^ 0xff000000);
            }
            dst_ptr += m_width;
        }
        #endif
    }
};




static SIZE get_window_frame_size(HWND hwnd) {
    WINDOWINFO wi;
    wi.cbSize = sizeof(wi);
    GetWindowInfo(hwnd, &wi);

    RECT rc = {0,0,0,0};
    AdjustWindowRectEx(&rc, wi.dwStyle, FALSE, wi.dwExStyle);

    if (wi.dwStyle & WS_VSCROLL) {
        rc.right += GetSystemMetrics(SM_CXVSCROLL);
    }

    SIZE sz = { rc.right-rc.left, rc.bottom-rc.top };
    return sz;
}


//----------------------------------------------------------------------------

class IWindowNotify_{
public:
    STDMETHOD(OnClosed)() = 0;
    STDMETHOD(OnKeyDown)(DWORD vk) = 0;
    STDMETHOD(OnMouseDown)(LONG button, LONG x, LONG y, LONG modifier, VARIANT_BOOL *presult) = 0;
    STDMETHOD(OnMouseUp)(LONG button, LONG x, LONG y, LONG modifier, VARIANT_BOOL *presult) = 0;
    STDMETHOD(OnMouseMove)(LONG button, LONG x, LONG y, LONG modifier, VARIANT_BOOL *presult) = 0;
    STDMETHOD(OnMouseWheel)(LONG x, LONG y, LONG delta) = 0;
    STDMETHOD(OnMenuInit)(HMENU menu) = 0;
    STDMETHOD(OnMenuExec)(DWORD id) = 0;
    STDMETHOD(OnTitleInit)() = 0;
    STDMETHOD(OnDrop)(BSTR bs, int type, DWORD key) = 0;
};

//----------------------------------------------------------------------------

class Window_{
    static const int TIMERID_MOUSE  = 0x674;
    static const int TIMERID_CURSOR = 0x675;
    inline void set_mouse_timer()  {  SetTimer(m_hwnd, TIMERID_MOUSE, 66, 0); }
    inline void kill_mouse_timer() { KillTimer(m_hwnd, TIMERID_MOUSE); }
    inline void set_cursor_timer() {  SetTimer(m_hwnd, TIMERID_CURSOR, 66, 0); }
    inline void kill_cursor_timer() { KillTimer(m_hwnd, TIMERID_CURSOR); }

    enum CID{
        CID_FG = 256,
        CID_BG,
        CID_Select,
        CID_Cursor,
        CID_ImeCursor,
        CID_Max,
    };

    IPty*    m_pty;
    IWindowNotify_* const  m_notify;
    HWND    m_hwnd;
    int    m_win_posx;
    int    m_win_posy;
    int    m_char_width;
    int    m_char_height;
    HDC    m_bmp_dc;
    OsBmp2    m_osbmp;
    Snapshot* m_snapshot;
    HFONT    m_font_handle[4];
    int    m_font_width;
    int    m_font_height;
    LOGFONT m_font_log;
    MouseCmd  m_lbtn_cmd;
    MouseCmd  m_mbtn_cmd;
    MouseCmd  m_rbtn_cmd;
    WinTransp m_transp_mode;
    WinZOrder m_zorder;
    RECT    m_border;
    int    m_linespace;
    int    m_vscrl_mode;
    SCROLLINFO m_vscrl;
    int    m_wheel_delta;
    VARIANT_BOOL m_blink_cursor;
    BYTE    m_cursor_step;
    bool    m_is_active;
    VARIANT_BOOL m_ime_on;
    //
    DWORD    m_colors[CID_Max];
    BYTE    m_fg_alpha;
    BYTE    m_bg_alpha;
    //
    bool    m_click;
    BYTE    m_click_count;
    MouseCmd m_click_cmd;
    DWORD    m_click_time;
    int    m_click_posx;
    int    m_click_posy;
    int    m_click_dx;
    int    m_click_dy;
    //
    int    m_select_x1;
    int    m_select_y1;
    int    m_select_x2;
    int    m_select_y2;
    //
protected:
    void draw_text(RECT& rc, CharFlag style, WCHAR* ws, INT* wi, int wn);
    void draw_screen(bool force_redraw);
    void draw_cursor();

    HRESULT _setup_font(LPCWSTR name, int height) {
        if (m_font_handle[0]) DeleteObject(m_font_handle[0]);
        if (m_font_handle[1]) DeleteObject(m_font_handle[1]);
        if (m_font_handle[2]) DeleteObject(m_font_handle[2]);
        if (m_font_handle[3]) DeleteObject(m_font_handle[3]);
        if (name != m_font_log.lfFaceName)
            StringCbCopy(m_font_log.lfFaceName, sizeof(m_font_log.lfFaceName), name);
        if (height<0) {
            if (-6 < height) height=-6;
        } else {
            if (height < 6) height=6;
        }
        m_font_log.lfHeight = height;
        m_font_log.lfWeight = FW_BOLD;
        m_font_log.lfUnderline = 1;
        m_font_handle[3] = CreateFontIndirect(&m_font_log);
        m_font_log.lfWeight = FW_NORMAL;
        m_font_handle[2] = CreateFontIndirect(&m_font_log);
        m_font_log.lfWeight = FW_BOLD;
        m_font_log.lfUnderline = 0;
        m_font_handle[1] = CreateFontIndirect(&m_font_log);
        m_font_log.lfWeight = FW_NORMAL;
        m_font_handle[0] = CreateFontIndirect(&m_font_log);
        //
        HGDIOBJ oldfont = SelectObject(m_bmp_dc, m_font_handle[0]);
        {
            TEXTMETRIC met;
            GetTextMetrics(m_bmp_dc, &met);
            ABC abc[52];
            GetCharABCWidths(m_bmp_dc, 'A','Z', abc+0);
            GetCharABCWidths(m_bmp_dc, 'a','z', abc+26);
            FLOAT width = 0.0f;
            for (int i=0; i < 52; i++) {
                width += abc[i].abcA;
                width += abc[i].abcB;
                width += abc[i].abcC;
            }
            m_font_width = (int)(width / 52.0f + 0.9f); 
            m_font_height = met.tmHeight;
        }
        SelectObject(m_bmp_dc, oldfont);
        //
        Resize(m_char_width, m_char_height);
        //
        HIMC imc = ImmGetContext(m_hwnd);
        if (imc) {
            ImmSetCompositionFont(imc, &m_font_log);
            ImmReleaseContext(m_hwnd,imc);
        }
        return S_OK;
    }
    void popup_menu(bool curpos) {
        POINT pt;
        if (curpos) {
            GetCursorPos(&pt);
        }
        else {
            pt.x=0;
            pt.y=-1;
            ClientToScreen(m_hwnd,&pt);
        }
        HMENU menu = CreatePopupMenu();
        m_notify->OnMenuInit((HMENU)menu);
        //
        DWORD id = (DWORD) TrackPopupMenuEx(menu, TPM_LEFTALIGN|TPM_TOPALIGN|TPM_LEFTBUTTON|TPM_RETURNCMD, pt.x, pt.y, m_hwnd, 0);
        DestroyMenu(menu);
        //
        m_notify->OnMenuExec(id);
    }

    void ime_set_state() {
        HIMC imc = ImmGetContext(m_hwnd);
        if (imc) {
            ImmSetOpenStatus(imc, m_ime_on);
            ImmSetCompositionFont(imc, &m_font_log);
            ImmReleaseContext(m_hwnd, imc);
        }
    }
    void ime_set_position() {
        if (m_pty) {
            HIMC imc = ImmGetContext(m_hwnd);
            if (imc) {
                int x,y;
                m_pty->get_CursorPosX(&x);
                m_pty->get_CursorPosY(&y);
                x = m_border.left + 0 + (x * (m_font_width + 0));
                y = m_border.top  + (m_linespace>>1) + (y * (m_font_height + m_linespace));
                COMPOSITIONFORM cf;
                cf.dwStyle = CFS_POINT;
                cf.ptCurrentPos.x = x;
                cf.ptCurrentPos.y = y;
                ImmSetCompositionWindow(imc,&cf);
                ImmReleaseContext(m_hwnd,imc);
            }
        }
    }
    void ime_send_result_string() {
        if (m_pty) {
            HIMC imc = ImmGetContext(m_hwnd);
            if (imc) {
                LONG n = ImmGetCompositionString(imc, GCS_RESULTSTR, 0, 0);
                if (n>0) {
                    BSTR bs = SysAllocStringByteLen(0, n+sizeof(WCHAR));
                    if (bs) {
                        n = ImmGetCompositionString(imc, GCS_RESULTSTR, bs, n);
                        if (n>0) {
                            bs[n/sizeof(WCHAR)] = '\0';
                            ((UINT*)bs)[-1] = n;
                            m_pty->PutString(bs);
                        }
                        SysFreeString(bs);
                    }
                }
                ImmReleaseContext(m_hwnd,imc);
            }
        }
    }
    //
    static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        Window_* p;
        if (msg==WM_CREATE) {
            p = (Window_*) ((CREATESTRUCT*)lp)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p);
        }
        else {
            p = (Window_*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }
        return (p)? p->wm_on_message(hwnd,msg,wp,lp): DefWindowProc(hwnd,msg,wp,lp);
    }
    LRESULT wm_on_message(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        switch(msg) {
        case WM_NCDESTROY:
            kill_cursor_timer();
            RevokeDragDrop(m_hwnd);
            DefWindowProc(hwnd,msg,wp,lp);
            trace("Window_::wm_on_nc_destroy\n");
            m_hwnd = 0;
            m_notify->OnClosed();
            return 0;
        case WM_CREATE:
            m_hwnd = hwnd;
            wm_on_create();
            break;
        case WM_ACTIVATE:
            if (LOWORD(wp) == WA_INACTIVE) {
                m_is_active = false;
                kill_cursor_timer();
                m_cursor_step = 0;
                m_wheel_delta = 0;
                draw_cursor();
            }
            else {//active
                m_is_active = true;
                m_cursor_step = 0;
                draw_cursor();
                if (m_blink_cursor)
                    set_cursor_timer();
                ime_set_state();
                if (m_pty) {
                    m_pty->Resize(m_char_width, m_char_height);
                }
            }
            break;
        case WM_GETMINMAXINFO:     wm_on_get_min_max_info((MINMAXINFO*)lp);return 0;
        case WM_WINDOWPOSCHANGING: wm_on_window_pos_changing((WINDOWPOS*)lp);return 0;
        case WM_WINDOWPOSCHANGED:  wm_on_window_pos_changed ((WINDOWPOS*)lp);return 0;
        case WM_TIMER:       wm_on_timer((DWORD)wp);break;
        case WM_MOUSEMOVE:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseMove(3, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_move(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
                }
            }
            return 0;
        case WM_LBUTTONDOWN:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseDown(0, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_down(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), m_lbtn_cmd);
                }
            }
            return 0;
        case WM_LBUTTONUP:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseUp(0, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_up(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), m_lbtn_cmd);
                }
            }
            return 0;
        case WM_MBUTTONDOWN:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseDown(1, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_down(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), m_mbtn_cmd);
                }
            }
            return 0;
        case WM_MBUTTONUP:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseUp(1, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_up(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), m_mbtn_cmd);
                }
            }
            return 0;
        case WM_RBUTTONDOWN:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseDown(2, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_down(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), m_rbtn_cmd);
                }
            }
            return 0;
        case WM_RBUTTONUP:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                VARIANT_BOOL result = VARIANT_FALSE;
                m_notify->OnMouseUp(2, x, y, ModKey_Mask, &result);
                if (result == VARIANT_FALSE) {
                    wm_on_mouse_up(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), m_rbtn_cmd);
                }
            }
            return 0;

        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            wm_on_paint();
            return 0;
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            m_cursor_step = 0;
            m_notify->OnKeyDown((DWORD)wp);
            return 0;
        case WM_MOUSEWHEEL:
            {
                int x = (GET_X_LPARAM(lp) - m_border.left) / (m_font_width + 0);
                int y = (GET_Y_LPARAM(lp) - m_border.top ) / (m_font_height + m_linespace);

                m_wheel_delta += GET_WHEEL_DELTA_WPARAM(wp);
                int notch = m_wheel_delta / WHEEL_DELTA;
                if (notch) {
                    m_wheel_delta -= notch * WHEEL_DELTA;
                    m_notify->OnMouseWheel(x, y, notch);
                }
            }
            return 0;
        case WM_INITMENUPOPUP:
            if (HIWORD(lp)) {
                UINT nb=GetMenuItemCount((HMENU)wp);
                for (UINT i=0; i<nb; i++) {
                    if (GetMenuItemID((HMENU)wp, i)==SC_CLOSE) {
                        for (UINT pos=(i+=2); i<nb; i++)
                            DeleteMenu((HMENU)wp, pos, MF_BYPOSITION);
                        break;
                    }
                }
                m_notify->OnMenuInit((HMENU)wp);
            }
            break;
        case WM_SYSCOMMAND:
            if ((wp & 0xFFF0) < 0xF000) {
                m_notify->OnMenuExec((DWORD)wp);
                return 0;
            }
            break;
        case WM_CONTEXTMENU:
            if (GET_X_LPARAM(lp)==-1 && GET_Y_LPARAM(lp)==-1) {
                popup_menu(false);
                return 0;
            }
            break;
        case WM_VSCROLL:
            wm_on_vscroll(LOWORD(wp));
            break;
        case WM_CHAR:
        case WM_SYSCHAR:
        case WM_IME_CHAR:
            return 0;
        case WM_IME_NOTIFY:
            if (wp == IMN_SETOPENSTATUS) {
                if (m_is_active) {
                    HIMC imc = ImmGetContext(m_hwnd);
                    if (imc) {
                        m_ime_on = ImmGetOpenStatus(imc)? TRUE: FALSE;
                        ImmReleaseContext(m_hwnd, imc);
                        draw_screen(true);
                    }
                }
            }
            break;
        case WM_IME_STARTCOMPOSITION:
            ime_set_position();
            break;
        case WM_IME_COMPOSITION:
            if (lp & GCS_CURSORPOS)
                ime_set_position();
            if (lp & GCS_RESULTSTR)
                ime_send_result_string();
            break;
        case WM_DWMCOMPOSITIONCHANGED:
            Util::set_window_transp(m_hwnd, m_transp_mode);
            break;
        case WM_USER_DROP_HDROP:
            if (wm_on_user_drop_hdrop((HGLOBAL)wp,(BOOL)lp))
                return 0;
            break;
        case WM_USER_DROP_WSTR:
            if (wm_on_user_drop_wstr((HGLOBAL)wp,(BOOL)lp))
                return 0;
            break;
        }
        return DefWindowProc(hwnd,msg,wp,lp);
    }
    void wm_on_create() {
        HMENU menu = GetSystemMenu(m_hwnd,FALSE);
        if (menu) {
            MENUITEMINFO mi;
            mi.cbSize = sizeof(mi);
            mi.fType = MFT_SEPARATOR;
            mi.fMask = MIIM_FTYPE;
            InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &mi);
        }
        if (m_vscrl_mode & 1) {
            SetScrollInfo(m_hwnd, SB_VERT, &m_vscrl, TRUE);
        }
        m_bmp_dc = CreateCompatibleDC(0);
        _setup_font(L"", 12);

        IDropTarget* p = new droptarget(m_hwnd);
        p->AddRef();
        RegisterDragDrop(m_hwnd, p);
        p->Release();
    }

    void wm_on_get_min_max_info(MINMAXINFO* p) {
        SIZE frame = get_window_frame_size(m_hwnd);
        p->ptMinTrackSize.x = frame.cx + (m_border.left + m_border.right) + (m_font_width + 0);
        p->ptMinTrackSize.y = frame.cy + (m_border.top + m_border.bottom) + (m_font_height + m_linespace);
    }
    void wm_on_window_pos_changing(WINDOWPOS* p) {
        if (m_zorder == WinZOrder_Bottom) {
            if (!(p->flags & SWP_NOZORDER))
                p->hwndInsertAfter = HWND_BOTTOM;
        }
        if (!(p->flags & SWP_NOSIZE) && !IsZoomed(m_hwnd)) {
            SIZE frame = get_window_frame_size(m_hwnd);
            int chaW = (p->cx - frame.cx - (m_border.left + m_border.right)) / (m_font_width  + 0);
            int chaH = (p->cy - frame.cy - (m_border.top + m_border.bottom)) / (m_font_height + m_linespace);
            int cliW = (m_border.left + m_border.right) + (chaW * (m_font_width  + 0));
            int cliH = (m_border.top + m_border.bottom) + (chaH * (m_font_height + m_linespace));
            int winW = (cliW + frame.cx);
            int winH = (cliH + frame.cy);
            if (!(p->flags & SWP_NOMOVE)) {
                if (p->x != m_win_posx) p->x += (p->cx - winW);
                if (p->y != m_win_posy) p->y += (p->cy - winH);
            }
            p->cx = winW;
            p->cy = winH;
        }
    }
    void wm_on_window_pos_changed(WINDOWPOS* p) {
        if (!(p->flags & SWP_NOSIZE)) {
            SIZE frame = get_window_frame_size(m_hwnd);
            int chaW = (p->cx - frame.cx - (m_border.left + m_border.right)) / (m_font_width  + 0);
            int chaH = (p->cy - frame.cy - (m_border.top + m_border.bottom)) / (m_font_height + m_linespace);
            int cliW,cliH;
            if (IsZoomed(m_hwnd)) {
                cliW = p->cx - frame.cx;
                cliH = p->cy - frame.cy;
            }
            else {
                cliW = (m_border.left + m_border.right) + (chaW * (m_font_width  + 0));
                cliH = (m_border.top + m_border.bottom) + (chaH * (m_font_height + m_linespace));
                int winW = (cliW + frame.cx);
                int winH = (cliH + frame.cy);
                if (winW != p->cx || winH != p->cy) {
                    if (!(p->flags & SWP_NOMOVE)) {
                        if (p->x != m_win_posx) p->x += (p->cx - winW);
                        if (p->y != m_win_posy) p->y += (p->cy - winH);
                        SetWindowPos(m_hwnd,0, p->x,p->y,winW,winH, SWP_NOZORDER|SWP_NOACTIVATE);
                    }
                    else {
                        SetWindowPos(m_hwnd,0, 0,0,winW,winH, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
                    }
                    return;
                }
            }
            if (m_char_width != chaW || m_char_height != chaH) {
                m_char_width  = chaW;
                m_char_height = chaH;
                if (m_pty) {
                    m_pty->Resize(chaW, chaH);
                }
            }
            if (m_osbmp.width() != cliW || m_osbmp.height() != cliH) {
                m_osbmp.Resize(cliW, cliH);
                m_osbmp.Fill(m_colors[CID_BG]);
                draw_screen(true);
            }
        }
        if (!(p->flags & SWP_NOMOVE)) {
            m_win_posx = p->x;
            m_win_posy = p->y;
        }
    }

    void wm_on_paint() {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(m_hwnd,&ps);
        if (m_osbmp.handle()) {
            HGDIOBJ old = SelectObject(m_bmp_dc, m_osbmp.handle());
            RECT& r = ps.rcPaint;
            BitBlt(dc, r.left,r.top, r.right-r.left, r.bottom-r.top, m_bmp_dc, r.left,r.top, SRCCOPY);
            SelectObject(m_bmp_dc, old);
        }
        EndPaint(m_hwnd,&ps);
    }

    void wm_on_vscroll(int req) {
        if (m_pty) {
            int n = 0;
            switch(req) {
            case SB_BOTTOM:   n=-1;break;
            case SB_TOP:      n=0;break;
            case SB_PAGEUP:   n=m_vscrl.nPos-m_vscrl.nPage; if (n<0)n=0; break;
            case SB_PAGEDOWN: n=m_vscrl.nPos+m_vscrl.nPage;break;
            case SB_LINEUP:   n=m_vscrl.nPos-1; if (n<0)n=0;break;
            case SB_LINEDOWN: n=m_vscrl.nPos+1;break;
            case SB_THUMBTRACK:
                {
                    SCROLLINFO si;
                    si.cbSize = sizeof(si);
                    si.fMask = SIF_TRACKPOS;
                    GetScrollInfo(m_hwnd, SB_VERT, &si);
                    n = si.nTrackPos;
                }
                break;
            default:
                return;
            }
            m_pty->put_ViewPos(n);
        }
    }

    void on_select_move(int x, int y, bool start) {
        if (m_pty) {
            int i;
            m_pty->get_ViewPos(&i);
            m_select_x2 = x;
            m_select_y2 = i+y;
            if (start) {
                m_select_x1 = x;
                m_select_y1 = i+y;
            }
            m_pty->SetSelection(m_select_x1,m_select_y1,m_select_x2,m_select_y2, m_click_count&3);
        }
    }
    void wm_on_mouse_down(int x, int y, MouseCmd cmd) {
        DWORD now = GetTickCount();
        DWORD st = (now>m_click_time) ? (now-m_click_time) : (now + ~m_click_time +1);
        int   sx = (x>m_click_posx)? (x-m_click_posx) : (m_click_posx-x);
        int   sy = (y>m_click_posy)? (y-m_click_posy) : (m_click_posy-y);
        if (cmd == m_click_cmd &&
           st <= GetDoubleClickTime() &&
           sx <= GetSystemMetrics(SM_CXDOUBLECLK) &&
           sy <= GetSystemMetrics(SM_CYDOUBLECLK))
            m_click_count++;
        else
            m_click_count=0;
        m_click_time = now;
        m_click_cmd = cmd;
        m_click = true;
        m_click_posx = x;
        m_click_posy = y;
        SetCapture(m_hwnd);
        m_click_dx = x = (x - m_border.left) / (m_font_width + 0);
        m_click_dy = y = (y - m_border.top ) / (m_font_height + m_linespace);
        if (m_click_cmd == MouseCmd_Select) {
            on_select_move(x, y, true);
            set_mouse_timer();
        }
        else if (m_click_cmd == MouseCmd_Paste) {
            m_select_y2 = 0;
            m_select_y1 = y;
            set_mouse_timer();
        }
        else if (m_click_cmd == MouseCmd_Menu) {
        }
    }
    void wm_on_timer(DWORD id) {
        if (id == TIMERID_MOUSE) {
            if (m_click && m_pty) {
                if (m_click_cmd == MouseCmd_Select) {
                    //trace("timer %d %d\n", m_select_x2,m_select_y2);
                    int n;
                    if (m_click_dy < 0) {
                        n = (m_click_dy);
                        if (n<-10) n=-10;
                    }
                    else if (m_click_dy >= m_char_height) {
                        n = (m_click_dy - m_char_height) +1;
                        if (n>10) n=10;
                    }
                    else {
                        return;
                    }
                    int i;
                    m_pty->get_ViewPos(&i);
                    n += i;
                    if (n<0)n=0;
                    if (n != i) {
                        m_pty->put_ViewPos(n);
                        m_pty->get_ViewPos(&n);
                        if (n != i) {
                            on_select_move(m_click_dx, m_click_dy, false);
                        }
                    }
                }
                else if (m_click_cmd == MouseCmd_Paste) {
                    int n = (m_click_dy - m_select_y1) / 2;
                    if (n<-10) n=-10; else if (n>10) n=10;
                    int i;
                    m_pty->get_ViewPos(&i);
                    n += i;
                    if (n<0)n=0;
                    if (n != i) {
                        m_pty->put_ViewPos(n);
                        m_pty->get_ViewPos(&n);
                    }
                }
            }
        }
        else if (id == TIMERID_CURSOR) {
            m_cursor_step++;
            draw_cursor();
        }
    }
    void wm_on_mouse_move(int x, int y) {
        if (m_click) {
            x = (x - m_border.left) / (m_font_width + 0);
            y = (y - m_border.top ) / (m_font_height + m_linespace);
            if (m_click_dx != x || m_click_dy != y) {
                m_click_dx = x;
                m_click_dy = y;
                if (m_click_cmd == MouseCmd_Select) {
                    on_select_move(x,y,false);
                }
                else if (m_click_cmd == MouseCmd_Paste) {
                    m_select_y2 = 1;
                }
                else if (m_click_cmd == MouseCmd_Menu) {
                }
            }
        }
    }
    void wm_on_mouse_up(int x, int y, MouseCmd cmd) {
        if (m_click && cmd == m_click_cmd) {
            m_click = false;
            ReleaseCapture();
            x = (x - m_border.left) / (m_font_width + 0);
            y = (y - m_border.top ) / (m_font_height + m_linespace);
            if (m_click_cmd == MouseCmd_Select) {
                kill_mouse_timer();
                if (m_click_dx != x || m_click_dy != y)
                    on_select_move(x,y,false);
                if (m_pty) {
                    BSTR bs;
                    m_pty->get_SelectedString(&bs);
                    if (bs) {
                        Util::set_clipboard_text(bs);
                        SysFreeString(bs);
                    }
                }
            }
            else if (m_click_cmd == MouseCmd_Paste) {
                kill_mouse_timer();
                if (m_select_y2==0) {
                    if (m_pty) {
                        BSTR bs = Util::get_clipboard_text();
                        if (bs) {
                            m_pty->PutString(bs);
                            SysFreeString(bs);
                        }
                    }
                }
            }
            else if (m_click_cmd == MouseCmd_Menu) {
                popup_menu(true);
            }
        }
    }

    bool wm_on_user_drop_hdrop(HGLOBAL mem, DWORD key) {
        if (mem) {
            HDROP drop = (HDROP) GlobalLock(mem);
            if (drop) {
                BSTR  bs = SysAllocStringLen(0, MAX_PATH+32);
                if (bs) {
                    DWORD nb = DragQueryFile(drop, (DWORD)-1, 0,0);
                    for (DWORD i=0; i<nb; i++) {
                        int len = (int) DragQueryFile(drop, i, bs, MAX_PATH+8);
                        if (len>0) {
                            bs[len] = '\0';
                            ((UINT*)bs)[-1] = len * sizeof(WCHAR);
                            m_notify->OnDrop(bs, 1, key);
                        }
                    }
                    SysFreeString(bs);
                }
                //DragFinish(drop);
                GlobalUnlock(mem);
                return true;
            }
        }
        return false;
    }
    bool wm_on_user_drop_wstr(HGLOBAL mem, DWORD key) {
        if (mem) {
            LPWSTR wstr = (LPWSTR) GlobalLock(mem);
            if (wstr) {
                BSTR bs = SysAllocString(wstr);
                if (bs) {
                    m_notify->OnDrop(bs, 0, key);
                }
                GlobalUnlock(mem);
                return true;
            }
        }
        return false;
    }

    //---

    void _finalize() {
        trace("Window_::dtor\n");
        if (m_pty) {
            if (m_snapshot) {
                m_pty->_del_snapshot(m_snapshot);
                m_snapshot = 0;
            }
            m_pty->Release();
        }
        if (m_hwnd)
            DestroyWindow(m_hwnd);
        if (m_font_handle[0])
            DeleteObject(m_font_handle[0]);
        if (m_font_handle[1])
            DeleteObject(m_font_handle[1]);
        if (m_font_handle[2])
            DeleteObject(m_font_handle[2]);
        if (m_font_handle[3])
            DeleteObject(m_font_handle[3]);
        if (m_bmp_dc)
            DeleteDC(m_bmp_dc);
    }
public:
    ~Window_() { _finalize(); }
    Window_(IWindowNotify_* cb)
        : m_pty(0),
          m_notify(cb),
          m_hwnd(0),
          m_win_posx(0),
          m_win_posy(0),
          m_char_width(80),
          m_char_height(24),
          m_bmp_dc(0),
          m_snapshot(0),
          m_font_width(0),
          m_font_height(0),
          m_lbtn_cmd(MouseCmd_Select),
          m_mbtn_cmd(MouseCmd_Paste),
          m_rbtn_cmd(MouseCmd_Menu),
          m_transp_mode(WinTransp_None),
          m_zorder(WinZOrder_Normal),
          m_linespace(0),
          m_vscrl_mode(1),
          m_wheel_delta(0),
          m_blink_cursor(TRUE),
          m_cursor_step(0),
          m_is_active(false),
          m_ime_on(FALSE),
          m_fg_alpha(0x44),
          m_bg_alpha(0x88),
          m_click(false),
          m_click_count(0),
          m_click_cmd(MouseCmd_None),
          m_click_time(0),
          m_click_posx(0),
          m_click_posy(0),
          m_click_dx(0),
          m_click_dy(0),
          m_select_x1(0),
          m_select_y1(0),
          m_select_x2(0),
          m_select_y2(0) {
        trace("Window_::ctor\n");
        memset(&m_font_handle, 0, sizeof(m_font_handle));
        memset(&m_font_log, 0, sizeof(m_font_log));
        m_font_log.lfCharSet = DEFAULT_CHARSET;
        m_font_log.lfOutPrecision = OUT_DEFAULT_PRECIS;
        m_font_log.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        m_font_log.lfQuality = DEFAULT_QUALITY;
        m_font_log.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
        memset(&m_border, 0, sizeof(m_border));
        memset(&m_vscrl, 0, sizeof(m_vscrl));
        m_vscrl.cbSize = sizeof(m_vscrl);
        m_vscrl.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        memset(&m_colors, 0, sizeof(m_colors));
        try{
            static const BYTE  table[]={0x00,0x5F,0x87,0xAF,0xD7,0xFE};
            static const DWORD color16[]={
                0x000000,0xCD0000,0x00CD00,0xCDCD00, 0x0000CD,0xCD00CD,0x00CDCD,0xE5E5E5,
                0x4D4D4D,0xFF0000,0x00FF00,0xFFFF00, 0x0000FF,0xFF00FF,0x00FFFF,0xFFFFFF,
            };
            int r,g,b, i=0;
            for (g=0; g<16; g++, i++) {
                m_colors[i] = color16[g];
            }
            for (r=0; r<6; r++) {
                for (g=0; g<6; g++) {
                    for (b=0; b<6; b++, i++) {
                        m_colors[i] = (table[r]<<16) | (table[g]<<8) | (table[b]);
                    }
                }
            }
            for (g=8; g<248; g+=10,i++) {
                m_colors[i] = (g<<16) | (g<<8) | (g);
            }
            m_colors[CID_FG]        = 0xff000000;
            m_colors[CID_BG]        = 0x88ffffff;
            m_colors[CID_Select]    = 0x440000ff;
            m_colors[CID_Cursor]    = 0xff00aa00;
            m_colors[CID_ImeCursor] = 0xffaa0000;

            static bool regist = false;
            LPCWSTR classname = L"ckWindowClass";
            if (!regist) {
                regist = true;
                WNDCLASSEX wc;
                memset(&wc, 0, sizeof(wc));
                wc.cbSize = sizeof(wc);
                wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
                wc.lpfnWndProc = wndproc;
                wc.hInstance = g_this_module;
                wc.hCursor = LoadCursor(0, IDC_ARROW);
                wc.hIcon = wc.hIconSm = LoadIcon(g_this_module, MAKEINTRESOURCE(1));
                wc.lpszClassName = classname;
                if (!RegisterClassEx(&wc))
                    throw (HRESULT) E_FAIL;
            }
            HWND hwnd = CreateWindowEx(0, classname, 0, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                CW_USEDEFAULT,CW_USEDEFAULT,0,0, 0,0,g_this_module,this);
            if (!hwnd) throw (HRESULT) E_FAIL;
        }
        catch(...) {
            _finalize();
            throw;
        }
    }

    STDMETHOD(get_Handle)(UINT* p) {
        *p = (UINT) m_hwnd;
        return S_OK;
    }
    STDMETHOD(get_Pty)(VARIANT* p) {
        if (m_pty) {
            m_pty->AddRef();
            p->pdispVal = m_pty;
            p->vt = VT_DISPATCH;
        }
        else {
            p->vt = VT_EMPTY;
        }
        return S_OK;
    }
    STDMETHOD(put_Pty)(VARIANT var) {
        if (m_pty) {
            if (m_snapshot) {
                m_pty->_del_snapshot(m_snapshot);
                m_snapshot = 0;
            }
            m_pty->Release();
            m_pty = 0;
        }
        if (var.vt == VT_DISPATCH && var.pdispVal) {
            var.pdispVal->QueryInterface(__uuidof(IPty), (void**)&m_pty);
        }
        if (m_pty) {
            if (m_is_active) {
                m_pty->Resize(m_char_width, m_char_height);
            }
            m_notify->OnTitleInit();
        }
        draw_screen(false);
        return S_OK;
    }
    STDMETHOD(put_Title)(BSTR bs) { SetWindowText(m_hwnd, bs); return S_OK; }
    STDMETHOD(get_MouseLBtn)(MouseCmd* p) { *p = m_lbtn_cmd; return S_OK; }
    STDMETHOD(get_MouseMBtn)(MouseCmd* p) { *p = m_mbtn_cmd; return S_OK; }
    STDMETHOD(get_MouseRBtn)(MouseCmd* p) { *p = m_rbtn_cmd; return S_OK; }
    STDMETHOD(put_MouseLBtn)(MouseCmd n) { m_lbtn_cmd=n; return S_OK; }
    STDMETHOD(put_MouseMBtn)(MouseCmd n) { m_mbtn_cmd=n; return S_OK; }
    STDMETHOD(put_MouseRBtn)(MouseCmd n) { m_rbtn_cmd=n; return S_OK; }
    STDMETHOD(get_Color)(int i, DWORD* p) {
        if (i < sizeof(m_colors) / sizeof(m_colors[0]))
            *p = m_colors[i];
        else
            *p = 0;
        return S_OK;
    }
    STDMETHOD(put_Color)(int i, DWORD color) {
        if (i < sizeof(m_colors)/sizeof(m_colors[0]))
            m_colors[i] = color;
        return S_OK;
    }
    STDMETHOD(get_ColorAlpha)(VARIANT_BOOL fg, BYTE* p) {
        *p = (fg) ? m_fg_alpha : m_bg_alpha;
        return S_OK;
    }
    STDMETHOD(put_ColorAlpha)(VARIANT_BOOL fg, BYTE n) {
        if (fg) m_fg_alpha = n;
        else   m_bg_alpha = n;
        return S_OK;
    }
    STDMETHOD(get_BackgroundImage)(BSTR* pp) {
        BSTR path = m_osbmp.GetBgFile();
        *pp = (path) ? SysAllocStringLen(path, SysStringLen(path)) : 0;
        return S_OK;
    }
    STDMETHOD(put_BackgroundImage)(BSTR cygpath) { m_osbmp.SetBgFile(cygpath); draw_screen(true); return S_OK; }
    STDMETHOD(get_BackgroundPlace)(DWORD* p) { *p = m_osbmp.GetBgPlace(); return S_OK; }
    STDMETHOD(put_BackgroundPlace)(DWORD n) { m_osbmp.SetBgPlace(n); draw_screen(true); return S_OK; }
    STDMETHOD(get_FontName)(BSTR* pp) {
        int len = (int)wcslen(m_font_log.lfFaceName);
        if (len<0)len=0;
        *pp = SysAllocStringLen(m_font_log.lfFaceName, len);
        return S_OK;
    }
    STDMETHOD(put_FontName)(BSTR p) { _setup_font(p, m_font_log.lfHeight); return S_OK; }
    STDMETHOD(get_FontSize)(int* p) { *p = m_font_log.lfHeight; return S_OK; }
    STDMETHOD(put_FontSize)(int n) { _setup_font(m_font_log.lfFaceName, n); return S_OK; }
    STDMETHOD(get_Linespace)(BYTE* p) { *p = (BYTE)m_linespace; return S_OK; }
    STDMETHOD(put_Linespace)(BYTE n) {
        if (m_linespace != n) {
            m_linespace = n;
            Resize(m_char_width, m_char_height);
        }
        return S_OK;
    }
    STDMETHOD(get_Border)(DWORD* p) {
        *p = (m_border.left<<24) | (m_border.top<<16) | (m_border.right<<8) | (m_border.bottom);
        return S_OK;
    }
    STDMETHOD(put_Border)(DWORD n) {
        m_border.left   = (n>>24) & 0xff;
        m_border.top    = (n>>16) & 0xff;
        m_border.right  = (n>>8 ) & 0xff;
        m_border.bottom = (n>>0 ) & 0xff;
        return S_OK;
    }
    STDMETHOD(get_Scrollbar)(int* p) { *p = m_vscrl_mode; return S_OK; }
    STDMETHOD(put_Scrollbar)(int n) {
        if (m_vscrl_mode != n) {
            m_vscrl_mode = n;
            DWORD style   = GetWindowLong(m_hwnd, GWL_STYLE);
            DWORD exstyle = GetWindowLong(m_hwnd, GWL_EXSTYLE);
            if (n & 1) style |= WS_VSCROLL;
            else  style &= ~WS_VSCROLL;
            if (n & 2) exstyle |= WS_EX_LEFTSCROLLBAR;
            else exstyle &= ~WS_EX_LEFTSCROLLBAR;
            SetWindowLong(m_hwnd, GWL_STYLE, style);
            SetWindowLong(m_hwnd, GWL_EXSTYLE, exstyle);
            if (n & 1) SetScrollInfo(m_hwnd, SB_VERT, &m_vscrl, FALSE);
            Resize(m_char_width, m_char_height);
        }
        return S_OK;
    }
    STDMETHOD(get_Transp)(WinTransp* p) { *p = m_transp_mode; return S_OK; }
    STDMETHOD(put_Transp)(WinTransp n) {
        if (m_transp_mode != n)
            m_transp_mode = Util::set_window_transp(m_hwnd,n);
        return S_OK;
    }
    STDMETHOD(get_ZOrder)(WinZOrder* p) { *p = m_zorder; return S_OK; }
    STDMETHOD(put_ZOrder)(WinZOrder n) {
        if (m_zorder != n) {
            m_zorder = n;
            HWND after;
            switch(m_zorder) {
            case WinZOrder_Top:   after=HWND_TOPMOST;break;
            case WinZOrder_Bottom:after=HWND_BOTTOM;break;
            default:              after=HWND_NOTOPMOST;break;
            }
            SetWindowPos(m_hwnd, after,    0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
            SetWindowPos(m_hwnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
        }
        return S_OK;
    }
    STDMETHOD(get_BlinkCursor)(VARIANT_BOOL* p) { *p = m_blink_cursor; return S_OK; }
    STDMETHOD(put_BlinkCursor)(VARIANT_BOOL b) {
        if (m_blink_cursor != b) {
            m_blink_cursor=b;
            if (m_blink_cursor) {
                set_cursor_timer();
            }
            else {
                m_cursor_step = 0;
                draw_cursor();
                kill_cursor_timer();
            }
        }
        return S_OK;
    }
    STDMETHOD(get_ImeState)(VARIANT_BOOL* p) { *p = m_ime_on; return S_OK; }
    STDMETHOD(put_ImeState)(VARIANT_BOOL b) {
        m_ime_on = b;
        ime_set_state();
        return S_OK;
    }
    STDMETHOD(Close)() { PostMessage(m_hwnd, WM_CLOSE, 0,0); return S_OK; }
    STDMETHOD(Show)() { Util::show_window(m_hwnd, SW_SHOW); return S_OK; }
    STDMETHOD(Resize)(int cw, int ch) {
        SIZE frame = get_window_frame_size(m_hwnd);
        cw = frame.cx + (m_border.left + m_border.right) + (cw * (m_font_width  + 0));
        ch = frame.cy + (m_border.top + m_border.bottom) + (ch * (m_font_height + m_linespace));
        SetWindowPos(m_hwnd,0, 0,0,cw,ch, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
        return S_OK;
    }
    STDMETHOD(Move)(int x, int y) {
        RECT  workarea;
        SystemParametersInfo(SPI_GETWORKAREA,0,(LPVOID)&workarea,0);
        RECT rc;
        GetWindowRect(m_hwnd, &rc);
        x = (x<0) ? (workarea.right+1-(rc.right-rc.left)+x)  : (workarea.left+x);
        y = (y<0) ? (workarea.bottom+1-(rc.bottom-rc.top)+y) : (workarea.top+y);
        SetWindowPos(m_hwnd,0, x,y,0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
        return S_OK;
    }
    STDMETHOD(PopupMenu)(VARIANT_BOOL system_menu) {
        if (system_menu)
            PostMessage(m_hwnd, WM_SYSCOMMAND, SC_KEYMENU, VK_SPACE);
        else
            PostMessage(m_hwnd, WM_CONTEXTMENU, 0,MAKELPARAM(-1,-1));
        return S_OK;
    }
    STDMETHOD(UpdateScreen)() { draw_screen(false); return S_OK; }
};


//----------------------------------------------------------------------------

void Window_::draw_text(RECT& rect, CharFlag style, WCHAR* ws, INT* wi, int wn) {
    {
        HFONT fn;
        switch(style & (CharFlag_Bold | CharFlag_Uline)) {
        case CharFlag_Bold | CharFlag_Uline:  fn=m_font_handle[3];break;
        case CharFlag_Uline:  fn=m_font_handle[2];break;
        case CharFlag_Bold:  fn=m_font_handle[1];break;
        default:  fn=m_font_handle[0];break;
        }
        SelectObject(m_bmp_dc, fn);
    }

    DWORD fg, bg, cursfg, cursbg;
    int   cid_fg = (style & CharFlag_FG) ? ((style>>16)&0xff) : CID_FG;
    int   cid_bg = (style & CharFlag_BG) ? ((style>>24)&0xff) : CID_BG;
    if (style & CharFlag_Invert) {
        int bak = cid_fg;
        cid_fg = cid_bg;
        cid_bg = bak;
    }
    if (style & CharFlag_Bold) {
        if (cid_fg <= 7)
            cid_fg += 8;
    }

    fg = m_colors[cid_fg] | 0xff000000;
    bg = m_colors[CID_BG];
    if (cid_bg != CID_BG) {
        bg = composite_argb(bg, m_colors[cid_bg] & 0x00ffffff | (m_bg_alpha<<24));
    }

    if (style & CharFlag_Cursor) {
        int alpha = (m_cursor_step & 8) ? m_cursor_step: ~m_cursor_step;
        alpha = (alpha & 7) * (255/7);

        DWORD c = m_colors[ m_ime_on ? CID_ImeCursor : CID_Cursor ] & 0x00ffffff | (alpha<<24);

        cursfg = composite_argb(fg, c ^ 0x00ffffff);
        cursbg = composite_argb(bg, c);
        if (style & CharFlag_Select) {
            cursfg = composite_argb(cursfg, m_colors[CID_Select]);
            cursbg = composite_argb(cursbg, m_colors[CID_Select]);
        }
        if (m_is_active) {
            fg = cursfg;
            bg = cursbg;
        }
    }
    if (style & CharFlag_Select) {
        fg = composite_argb(fg, m_colors[CID_Select]);
        bg = composite_argb(bg, m_colors[CID_Select]);
    }

    int x = rect.left;
    int y = rect.top;
    RECT rc = rect;
    if (rc.left < 0 + m_border.left) rc.left = 0 + m_border.left;
    if (rc.top  < 0 + m_border.top ) rc.top  = 0 + m_border.top;
    if (rc.right  > m_osbmp.width()  - m_border.right)  rc.right  = m_osbmp.width()  - m_border.right;
    if (rc.bottom > m_osbmp.height() - m_border.bottom) rc.bottom = m_osbmp.height() - m_border.bottom;
    if (rc.left >= rc.right || rc.top >= rc.bottom) return;

    if (m_is_active || !(style & CharFlag_Cursor)) {
        m_osbmp.Text_fill(rc, bg);
    }
    else {
        RECT  r2 = {rc.left+1, rc.top+1, rc.right-1, rc.bottom-1};
        m_osbmp.Text_rect(rc, cursbg);
        m_osbmp.Text_fill(r2, bg);
    }
    SetTextColor(m_bmp_dc, (fg&0xff00) | ((fg&0xff)<<16) | ((fg>>16)&0xff));
    ExtTextOut(m_bmp_dc, x,y, ETO_CLIPPED, &rc, ws,wn,wi);
    if (m_fg_alpha) m_osbmp.Text_glow(rc, m_fg_alpha);
    m_osbmp.Text_to_pargb(rc);
    InvalidateRect(m_hwnd, &rc, FALSE);
}

void Window_::draw_screen(bool force_redraw) {
    if (!m_pty) return;
    if (!m_osbmp.pixels()) return;

    Snapshot* now=0;
    m_pty->_new_snapshot(&now);
    if (!now)return;

    Char*    nowchars = now->chars;
    Char*    oldchars = 0;

    if (!force_redraw &&
       m_snapshot &&
       m_snapshot->width == now->width &&
       m_snapshot->height == now->height) {
        oldchars = m_snapshot->chars;
    }

    HGDIOBJ gdi_oldbmp  = SelectObject(m_bmp_dc, m_osbmp.handle());
    HGDIOBJ    gdi_oldfont = SelectObject(m_bmp_dc, GetStockObject(DEFAULT_GUI_FONT));

    int    maxCW = (m_char_width  < now->width)  ? m_char_width  : now->width;
    int    maxCH = (m_char_height < now->height) ? m_char_height : now->height;
    int    lineH = m_font_height + m_linespace;
    WCHAR*    ws = new WCHAR[maxCW];
    INT*    wi = new INT[maxCW];
    int    wn = 0;
    RECT    rc;
    CharFlag style = (CharFlag)0;

    rc.top = 0 + m_border.top + (m_linespace>>1);

    for (int y=0; y < maxCH; y++) {
        rc.left = rc.right = m_border.left;
        rc.bottom = rc.top + m_font_height;

        if (oldchars && memcmp(oldchars, nowchars, sizeof(Char) * maxCW) == 0) {
        }
        else {
            for (int x=0; x < maxCW; x++) {
                Char& c = nowchars[x];

                if (c.flags & CharFlag_MbTrail) {
                    if (x>0)continue;
                    rc.left = rc.right = -m_font_width;
                }
                CharFlag s = (CharFlag)(c.flags & CharFlag_Styles);
                if (style != s) {
                    if (wn > 0) {
                        draw_text(rc, style, ws, wi, wn);
                        wn=0;
                        rc.left=rc.right;
                    }
                    style = s;
                }

                ws[wn] = (c.ch < 0x20) ? ' ' : c.ch;
                wi[wn] = (c.flags & (CharFlag_MbLead|CharFlag_MbTrail)) ? (m_font_width<<1) : (m_font_width);
                rc.right += wi[wn];
                wn++;
            }
            if (wn > 0) {
                draw_text(rc, style, ws, wi, wn);
                wn=0;
            }
        }

        rc.top += m_font_height + m_linespace;

        if (oldchars) oldchars += now->width;
        nowchars += now->width;
    }

    delete [] ws;
    delete [] wi;
    SelectObject(m_bmp_dc, gdi_oldfont);
    SelectObject(m_bmp_dc, gdi_oldbmp);

    if (m_snapshot)
        m_pty->_del_snapshot(m_snapshot);
    m_snapshot = now;

    if (m_vscrl.nPage != now->height ||
       m_vscrl.nPos  != now->view ||
       m_vscrl.nMax  != now->nlines-1) {
        m_vscrl.nPage = now->height;
        m_vscrl.nPos  = now->view;
        m_vscrl.nMax  = now->nlines-1;
        if (m_vscrl_mode & 1) {
            SetScrollInfo(m_hwnd,SB_VERT,&m_vscrl,TRUE);
        }
    }

    UpdateWindow(m_hwnd);
}

void Window_::draw_cursor() {
    if (m_snapshot) {
        int   x = m_snapshot->cursor_x;
        int   y = m_snapshot->cursor_y;
        if (x >= 0) {
            Char* chars = m_snapshot->chars + (m_snapshot->width * y) + x;

            HGDIOBJ gdi_oldbmp  = SelectObject(m_bmp_dc, m_osbmp.handle());
            HGDIOBJ    gdi_oldfont = SelectObject(m_bmp_dc, GetStockObject(DEFAULT_GUI_FONT));
            SetBkMode(m_bmp_dc, TRANSPARENT);

            RECT rc;
            rc.left   = m_border.left + (m_font_width * x);
            rc.right  = rc.left + m_font_width;
            rc.top    = m_border.top + (m_linespace>>1) + ((m_font_height + m_linespace) * y);
            rc.bottom = rc.top + m_font_height;

            WCHAR ch = (chars->ch < 0x20) ? ' ' : chars->ch;
            INT   w;
            if (chars->flags & CharFlag_MbLead) {
                rc.right += m_font_width;
                w = m_font_width << 1;
            }
            else if (chars->flags & CharFlag_MbTrail) {
                rc.left -= m_font_width;
                w = m_font_width << 1;
            }
            else {
                w = m_font_width;
            }

            draw_text(rc, (CharFlag)(chars->flags & CharFlag_Styles), &ch, &w, 1);

            SelectObject(m_bmp_dc, gdi_oldfont);
            SelectObject(m_bmp_dc, gdi_oldbmp);
        }
    }
}

//----------------------------------------------------------------------------

class Window: public IDispatchImpl3<IWnd>, public IWindowNotify_{
protected:
    Window_*  m_obj;
    IWndNotify* const  m_notify;
    //
    void _finalize() {
        if (m_obj) { delete m_obj; m_obj=0; }
    }
    virtual ~Window() {
        trace("Window::dtor\n");
        _finalize();
    }
public:
    Window(IWndNotify* cb): m_obj(0), m_notify(cb) {
        trace("Window::ctor\n");
        m_obj = new Window_(this);
    }

    //STDMETHOD_(ULONG,AddRef)() { ULONG n=IDispatchImpl3<IWnd>::AddRef(); trace("Window::AddRef %d\n", n); return n; }
    //STDMETHOD_(ULONG,Release)() { ULONG n=IDispatchImpl3<IWnd>::Release(); trace("Window::Release %d\n", n); return n; }

    STDMETHOD(OnClosed)() { return m_notify->OnClosed(this); }
    STDMETHOD(OnKeyDown)(DWORD vk) { return m_notify->OnKeyDown(this, vk); }
    STDMETHOD(OnMouseDown)(LONG button, LONG x, LONG y, LONG modifier, VARIANT_BOOL *presult) { 
        return m_notify->OnMouseDown(this, button, x, y, modifier, presult);
    }
    STDMETHOD(OnMouseUp)(LONG button, LONG x, LONG y, LONG modifier, VARIANT_BOOL *presult) { 
        return m_notify->OnMouseUp(this, button, x, y, modifier, presult);
    }
    STDMETHOD(OnMouseMove)(LONG button, LONG x, LONG y, LONG modifier, VARIANT_BOOL *presult) { 
        return m_notify->OnMouseMove(this, button, x, y, modifier, presult);
    }
    STDMETHOD(OnMouseWheel)(LONG x, LONG y, LONG delta) { return m_notify->OnMouseWheel(this, x, y, delta); }
    STDMETHOD(OnMenuInit)(HMENU menu) { return m_notify->OnMenuInit(this, (int*)menu); }
    STDMETHOD(OnMenuExec)(DWORD id) { return m_notify->OnMenuExec(this, id); }
    STDMETHOD(OnTitleInit)() { return m_notify->OnTitleInit(this); }
    STDMETHOD(OnDrop)(BSTR bs, int type, DWORD key) { return m_notify->OnDrop(this,bs,type,key); }
    //
    STDMETHOD(Dispose)() {
        trace("Window::dispose\n");
        _finalize();
        return S_OK;
    }
    //
    STDMETHOD(get_Handle)(UINT* p) { return m_obj->get_Handle(p); }
    STDMETHOD(get_Pty)(VARIANT* p) { return m_obj->get_Pty(p); }
    STDMETHOD(put_Pty)(VARIANT var) { return m_obj->put_Pty(var); }
    STDMETHOD(put_Title)(BSTR p) { return m_obj->put_Title(p); }
    STDMETHOD(get_MouseLBtn)(MouseCmd* p) { return m_obj->get_MouseLBtn(p); }
    STDMETHOD(put_MouseLBtn)(MouseCmd n) { return m_obj->put_MouseLBtn(n); }
    STDMETHOD(get_MouseMBtn)(MouseCmd* p) { return m_obj->get_MouseMBtn(p); }
    STDMETHOD(put_MouseMBtn)(MouseCmd n) { return m_obj->put_MouseMBtn(n); }
    STDMETHOD(get_MouseRBtn)(MouseCmd* p) { return m_obj->get_MouseRBtn(p); }
    STDMETHOD(put_MouseRBtn)(MouseCmd n) { return m_obj->put_MouseRBtn(n); }
    STDMETHOD(get_Color)(int i, DWORD* p) { return m_obj->get_Color(i,p); }
    STDMETHOD(put_Color)(int i, DWORD color) { return m_obj->put_Color(i,color); }
    STDMETHOD(get_ColorAlpha)(VARIANT_BOOL fg, BYTE* p) { return m_obj->get_ColorAlpha(fg, p); }
    STDMETHOD(put_ColorAlpha)(VARIANT_BOOL fg, BYTE n) { return m_obj->put_ColorAlpha(fg, n); }
    STDMETHOD(get_BackgroundImage)(BSTR* pp) { return m_obj->get_BackgroundImage(pp); }
    STDMETHOD(put_BackgroundImage)(BSTR path) { return m_obj->put_BackgroundImage(path); }
    STDMETHOD(get_BackgroundPlace)(DWORD* p) { return m_obj->get_BackgroundPlace(p); }
    STDMETHOD(put_BackgroundPlace)(DWORD n) { return m_obj->put_BackgroundPlace(n); }
    STDMETHOD(get_FontName)(BSTR* pp) { return m_obj->get_FontName(pp); }
    STDMETHOD(put_FontName)(BSTR p) { return m_obj->put_FontName(p); }
    STDMETHOD(get_FontSize)(int* p) { return m_obj->get_FontSize(p); }
    STDMETHOD(put_FontSize)(int n) { return m_obj->put_FontSize(n); }
    STDMETHOD(get_LineSpace)(BYTE* p) { return m_obj->get_Linespace(p); }
    STDMETHOD(put_LineSpace)(BYTE n) { return m_obj->put_Linespace(n); }
    STDMETHOD(get_Border)(DWORD* p) { return m_obj->get_Border(p); }
    STDMETHOD(put_Border)(DWORD n) { return m_obj->put_Border(n); }
    STDMETHOD(get_Scrollbar)(int* p) { return m_obj->get_Scrollbar(p); }
    STDMETHOD(put_Scrollbar)(int n) { return m_obj->put_Scrollbar(n); }
    STDMETHOD(get_Transp)(WinTransp* p) { return m_obj->get_Transp(p); }
    STDMETHOD(put_Transp)(WinTransp n) { return m_obj->put_Transp(n); }
    STDMETHOD(get_ZOrder)(WinZOrder* p) { return m_obj->get_ZOrder(p); }
    STDMETHOD(put_ZOrder)(WinZOrder n) { return m_obj->put_ZOrder(n); }
    STDMETHOD(get_BlinkCursor)(VARIANT_BOOL* p) { return m_obj->get_BlinkCursor(p); }
    STDMETHOD(put_BlinkCursor)(VARIANT_BOOL b) { return m_obj->put_BlinkCursor(b); }
    STDMETHOD(get_ImeState)(VARIANT_BOOL* p) { return m_obj->get_ImeState(p); }
    STDMETHOD(put_ImeState)(VARIANT_BOOL b) { return m_obj->put_ImeState(b); }
    STDMETHOD(Close)() { return m_obj->Close(); }
    STDMETHOD(Show)() { return m_obj->Show(); }
    STDMETHOD(Resize)(int cw, int ch) { return m_obj->Resize(cw,ch); }
    STDMETHOD(Move)(int x, int y) { return m_obj->Move(x,y); }
    STDMETHOD(PopupMenu)(VARIANT_BOOL system_menu) { return m_obj->PopupMenu(system_menu); }
    STDMETHOD(UpdateScreen)() { return m_obj->UpdateScreen(); }
};


}//namespace Ck

//----------------------------------------------------------------------------

extern "C" __declspec(dllexport) HRESULT CreateWnd(Ck::IWndNotify* cb, Ck::IWnd** pp) {
    if (!pp) return E_POINTER;
    if (!cb) return E_INVALIDARG;

    HRESULT hr;
    Ck::Window* w = 0;

    try{
        w = new Ck::Window(cb);
        w->AddRef();
        hr = S_OK;
    }
    catch(HRESULT e) {
        hr = e;
    }
    catch(std::bad_alloc&) {
        hr = E_OUTOFMEMORY;
    }
    catch(...) {
        hr = E_FAIL;
    }

    (*pp) = w;
    return hr;
}

//EOF
