/*----------------------------------------------------------------------------
 * Copyright 2004-2005,2007  Kazuo Ishii <k-ishii@wb4.so-net.ne.jp>
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
#include <WinNls.h>
//#import  "interface.tlb" raw_interfaces_only
#include "interface.tlh"
#include "encoding.h"


namespace Ck{
namespace Enc{

Encoding  detect_encoding(const BYTE* src, int srclen, Encoding mask){
	if(mask==Encoding_EUCJP ||
	   mask==Encoding_SJIS ||
	   mask==Encoding_UTF8)
		return mask;

	int sjis_count = 0;
	int utf8_count = 0;

	if((mask & Encoding_SJIS) && (mask & Encoding_EUCJP)){
		for(int i=0; i < srclen; i++){
			if((src[i] & 0x80) && (i+1 < srclen)){
				if(eucj_to_ucs((src[i]<<8) | src[i+1]))
					sjis_count--;
				else
					sjis_count++;
			}
		}
	}
	if(mask & Encoding_UTF8){//utf8
		for(int i=0; i < srclen; i++){
			if(src[i] & 0x80){
				if((src[i] & 0xF0)==0xE0){
					if(++i >= srclen)break;
					if((src[i] & 0xC0)==0x80){
						if(++i >= srclen)break;
						if((src[i] & 0xC0)==0x80){
							utf8_count++;
							continue;
						}
					}
				}
				else if((src[i] & 0xE0)==0xC0){
					if(++i >= srclen)break;
					if((src[i] & 0xC0)==0x80){
						utf8_count++;
						continue;
					}
				}
				utf8_count--;
			}
		}
	}

	if(utf8_count>0)
		return Encoding_UTF8;
	if(!(mask & Encoding_EUCJP) || sjis_count>0)
		return Encoding_SJIS;
	return Encoding_EUCJP;
}


/*
 * normaliz.dllは、Vistaに最初から入っている。
 * XPはIE7を入れると一緒に入る模様。
 *
 * これ入れれば良いのかな?
 * http://www.microsoft.com/downloads/details.aspx?familyid=AD6158D7-DDBA-416A-9109-07607425A815&displaylang=en
 *
 * ・素のXPはdllが無い
 * ・normalizeしなくても、あまり問題ない
 * ・WindowsSDKの normaliz.libが間違っていて 自前implibを作らないと静的リンクできないバグ
 *
 * ってことで動的リンクする。
 */
class nls{
private:
	typedef int (WINAPI * tIsNormalizedString)(NORM_FORM,LPCWSTR,int);
	typedef int (WINAPI * tNormalizeString)(NORM_FORM,LPCWSTR,int,LPWSTR,int);
	HMODULE  m_module;
	tIsNormalizedString  m_IsNormalizedString;
	tNormalizeString  m_NormalizeString;
public:
	~nls(){
		if(m_module) FreeLibrary(m_module);
	}
	nls():m_module(0), m_IsNormalizedString(0), m_NormalizeString(0){
		m_module = LoadLibrary(L"normaliz.dll");
		if(m_module){
			m_IsNormalizedString = (tIsNormalizedString) GetProcAddress(m_module, "IsNormalizedString");
			m_NormalizeString = (tNormalizeString) GetProcAddress(m_module, "NormalizeString");
			if(!m_IsNormalizedString || !m_NormalizeString){
				FreeLibrary(m_module);
				m_module = 0;
			}
		}
	}
	int  normalization_form_c(WCHAR* io_buffer, int in_length, int out_max){
		if(m_module && in_length > 0 && m_IsNormalizedString(NormalizationC, io_buffer, in_length)==FALSE){
			WCHAR* wk = new WCHAR [in_length];
			memcpy(wk, io_buffer, sizeof(WCHAR) * in_length);
			in_length = m_NormalizeString(NormalizationC, wk, in_length, io_buffer, out_max);
			delete [] wk;
		}
		return in_length;
	}
};
static nls  g_nls;


void utf8_to_ucs(const BYTE* src, int* srclen, WCHAR* dst, int* dstlen){
	int si=0, di=0;
	const int smax = *srclen;
	const int dmax = *dstlen;
	BYTE   b1,b2,b3;
	WCHAR  ucs;

	while(si < smax && di < dmax){
		b1 = src[si];
		if((b1 & 0xF0)==0xE0){
			if(si+1>=smax)break;
			b2 = src[si+1];
			if((b2 & 0xC0)==0x80){
				if(si+2>=smax)break;
				b3 = src[si+2];
				if((b3 & 0xC0)==0x80){
					ucs = ((b1 & 0x0F)<<12) | ((b2 & 0x3F)<<6) | ((b3 & 0x3F));
					if(ucs == 0xFEFF || ucs == 0xFFFE){//BOM
					}
					else{
						dst[di++] = ucs;
					}
					si += 3;
					continue;
				}
			}
		}
		else if((b1 & 0xE0)==0xC0){
			if(si+1>=smax)break;
			b2 = src[si+1];
			if((b2 & 0xC0)==0x80){
				ucs = ((b1 & 0x1F)<<6) | ((b2 & 0x3F));
				dst[di++] = ucs;
				si += 2;
				continue;
			}
		}
		si++;
		dst[di++] = b1;
	}

	di = g_nls.normalization_form_c(dst, di, dmax);

	*srclen = si;
	*dstlen = di;
}

void eucj_to_ucs(const BYTE* src, int* srclen, WCHAR* dst, int* dstlen){
	int si=0, di=0;
	const int smax = *srclen;
	const int dmax = *dstlen;

	while(di < dmax && si+1 < smax){
		BYTE  b = src[si];
		WCHAR ucs = eucj_to_ucs((b<<8)|src[si+1]);
		if(ucs){
			si+=2;
			dst[di++] = ucs;
		}
		else{
			si++;
			dst[di++] = eucj_to_ucs(b);
		}
	}
	if(di < dmax && si < smax){
		BYTE b = src[si];
		if((b==0x8E) || (0xA1<=b && b<=0xF4)){
		}
		else{
			si++;
			dst[di++] = eucj_to_ucs(b);
		}
	}
	*srclen = si;
	*dstlen = di;
}

void sjis_to_ucs(const BYTE* src, int* srclen, WCHAR* dst, int* dstlen){
	int si=0, di=0;
	const int smax = *srclen;
	const int dmax = *dstlen;

	while(di < dmax && si+1 < smax){
		BYTE  b = src[si];
		WCHAR ucs = sjis_to_ucs((b<<8)|src[si+1]);
		if(ucs){
			si+=2;
			dst[di++] = ucs;
		}
		else{
			si++;
			dst[di++] = sjis_to_ucs(b);
		}
	}
	if(di < dmax && si < smax){
		BYTE b = src[si];
		if((0x81<=b && b<=0x9F) || (0xE0<=b && b<=0xEA) || (0xF0<=b && b<=0xFC)){
		}
		else{
			si++;
			dst[di++] = sjis_to_ucs(b);
		}
	}
	*srclen = si;
	*dstlen = di;
}


static const unsigned int _is_dblchar_table []={
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0x7FFF03FF,0xFFFFFFFF,0xFFFFFFFF,
	0xF9FFFFFF,0xFFFFFFFF,0x07FFFFFF,0x00000000,0x00000000,0x00000000,0x00000000,0xFFFFF800,
	0xFFFFFFFF,0xFFFFFFFF,0x00000FFF,0x00000000,0x00000000,0x00000000,0x0003F800,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0xFA2C9000,0x080207BE,0x69BA1E0C,0x500012BC,
	0x70460040,0x6178BC08,0x00460000,0x00000000,0x00000000,0xAA000000,0x000000AA,0x00000000,
	0x00000000,0x10000000,0x00001000,0x00000000,0x00000000,0x09748000,0x00000578,0x00000000,
	0x00000000,0x00000000,0x00000000,0xF0000000,0xF01FDFFF,0x001FDFFF,0x00000000,0xF8001000,
	0xFFFFFFFF,0x37FFFFFF,0xA01EE66F,0x00000905,0x00000000,0x0003D002,0x02000000,0x00000000,
	0x00000000,0x00450000,0x0108C009,0x00000000,0xE1FFEF03,0xE000007F,0x0000007F,0x80000060,
	0x00100002,0x5131A000,0x0BF53C84,0x8220061E,0x199E6000,0x00198000,0x00040044,0x00001000,
	0x00000000,0x80000000,0x00C00000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0xFFFFE000,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFF7FFFFF,0xFFFFFFFF,0xFFFFFFFF,0xE1FFFFFF,0xFFFFFFFF,0x9FFFE001,0x807F6007,0x79386619,
	0x10078000,0x184C0000,0x00000A06,0x0000A000,0x16F76000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000400,0x00000000,0xFFFFFFF8,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
	0xFFFFFFFF,0xFFFFFFFF,0xE07FFFFF,0xFFFFEFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFF3F,0xFFFFFFFF,
	0x000000FF,0x00000000,0x00000000,0x00000000,0xFF000000,0xFFFFFFFF,0xFFFFFFFF,0x01FFFFFF,
	0x00000000,0x00000000,0x00000000,0x7F000000,0x00200000,
};

inline bool _is_dblchar(int idx){
	return ( _is_dblchar_table[idx>>5] & 1<<(idx&31) ) ? true : false;
}

bool is_dblchar(WCHAR n){
	if(n <= 0x309A){
		if(n <= 0x10FF) return false;
		if(n <= 0x115F) return true;
		if(n <= 0x2328) return false;
		if(n <= 0x232A) return true;
		if(n <= 0x2E7F) return false;
		if(n <= 0x309A) return _is_dblchar(n-0x2E80+0);
	}
	else{
		if(n <= 0xA4CF) return true;
		if(n <= 0xABFF) return false;
		if(n <= 0xD7A3) return true;
		if(n <= 0xF8FF) return false;
		if(n <= 0xFAFF) return true;
		if(n <= 0xFE2F) return false;
		if(n <= 0xFFFF) return _is_dblchar(n-0xFE30+539);
	}
	return false;
}

bool is_dblchar_cjk(WCHAR n){
	if(n <= 0x309A){
		if(n <= 0x0451) return _is_dblchar(n-0x0000+1003);
		if(n <= 0x10FF) return false;
		if(n <= 0x115F) return true;
		if(n <= 0x200F) return false;
		if(n <= 0x277F) return _is_dblchar(n-0x2010+2109);
		if(n <= 0x2E7F) return false;
		if(n <= 0x309A) return _is_dblchar(n-0x2E80+4013);
	}
	else{
		if(n <= 0xA4CF) return true;
		if(n <= 0xABFF) return false;
		if(n <= 0xD7A3) return true;
		if(n <= 0xDFFF) return false;
		if(n <= 0xFAFF) return true;
		if(n <= 0xFE2F) return false;
		if(n <= 0xFFFF) return _is_dblchar(n-0xFE30+4552);
	}
	return false;
}

}//namespace Enc
}//namespace Ck
//EOF
