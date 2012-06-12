#pragma once
namespace Ck{
namespace Enc{

struct _encoding_table_a{ WORD  e,s; const WCHAR* p; };
struct _encoding_table_u{ WCHAR e,s; const WORD*  p; };

inline WCHAR sjis_to_ucs (WORD n){
 extern const struct _encoding_table_a  _sjis_to_ucs_table [];
 const struct _encoding_table_a * t = _sjis_to_ucs_table + (n>>8);
 return (t->e >= n && n >= t->s) ? t->p[n - t->s] : 0;
}

inline WORD ucs_to_sjis (WCHAR n){
 extern const struct _encoding_table_u  _ucs_to_sjis_table [];
 const struct _encoding_table_u * t = _ucs_to_sjis_table + (n>>8);
 return (t->e >= n && n >= t->s) ? t->p[n - t->s] : 0;
}

inline WCHAR eucj_to_ucs (WORD n){
 extern const struct _encoding_table_a  _eucj_to_ucs_table [];
 const struct _encoding_table_a * t = _eucj_to_ucs_table + (n>>8);
 return (t->e >= n && n >= t->s) ? t->p[n - t->s] : 0;
}

inline WORD ucs_to_eucj (WCHAR n){
 extern const struct _encoding_table_u  _ucs_to_eucj_table [];
 const struct _encoding_table_u * t = _ucs_to_eucj_table + (n>>8);
 return (t->e >= n && n >= t->s) ? t->p[n - t->s] : 0;
}

Encoding  detect_encoding(const BYTE* src, int srclen, Encoding mask);

void utf8_to_ucs(const BYTE* src, int* srclen, WCHAR* dst, int* dstlen);
void eucj_to_ucs(const BYTE* src, int* srclen, WCHAR* dst, int* dstlen);
void sjis_to_ucs(const BYTE* src, int* srclen, WCHAR* dst, int* dstlen);

bool is_dblchar(WCHAR n);
bool is_dblchar_cjk(WCHAR n);

}//namespace Enc
}//namespace Ck
//EOF
