#pragma once
namespace Ck{

struct Line{
	Char*	chars;
	int	count;
	int	maxsize;
};

class Screen_{
private:
	Line*	m_buffer;
	int	m_maxsize;
	int	m_head;

	int	m_view_top;
	int	m_page_top;
	int	m_pageW;
	int	m_pageH;

	int	m_curX;
	int	m_curY;

	int	m_rgn_top;
	int	m_rgn_btm;

	int	m_sel_top;
	int	m_sel_topX;
	int	m_sel_btm;
	int	m_sel_btmX;

	int	m_save_curX;
	int	m_save_curY;
	int	m_savelines;
	int	m_feed_next;
	int	m_insert_mode;
	CharFlag m_style;

	Line& get_line(int pos){
		pos = m_head + m_page_top + pos;
		if(pos >= m_maxsize)
			pos -= m_maxsize;
		return m_buffer[pos];
	}
	Line& get_line_glb(int pos){
		pos = m_head + pos;
		if(pos >= m_maxsize)
			pos -= m_maxsize;
		return m_buffer[pos];
	}

	bool check_selection_x(int y, int l, int r);
	bool check_selection_y(int t, int b);
	void clear_line(Line& p, int start, int end);
	void _resize_line(Line& p);
	void _resize(int newsize);
	void scroll_up(int top, int btm, int n);
	void scroll_down(int top, int btm, int n);
	void add_char(WCHAR w, CharFlag style);

	void _finalize();
public:
	~Screen_(){ _finalize();}
	Screen_();
	void Resize(int w, int h);
	void ErasePage(int mode);
	void EraseLine(int mode);
	void SetRegion(int top, int btm);
	void ScrollPage(int n);
	void InsertLine(int n);
	void DeleteLine(int n);
	void Feed();
	void FeedRev();
	void InsertChar(int n);
	void DeleteChar(int n);
	void EraseChar(int n);

	int  GetSavelines(){ return m_savelines;}
	void SetSavelines(int n){
		if(n < 0) n=0;
		m_savelines = n;
		_resize(m_pageH + m_savelines);
		if(m_page_top > m_savelines){
			ClearSelection();
			m_head = m_head + m_page_top - m_savelines;
			if(m_head < 0)
				m_head += m_maxsize;
			if(m_head >= m_maxsize)
				m_head -= m_maxsize;
			m_page_top = m_view_top = (m_savelines);
		}
	}

	int  GetNumLines(){ return m_page_top+m_pageH; }
	int  GetViewPos(){ return m_view_top; }
	void SetViewPos(int n){
		if(n < 0 || n > m_page_top) n=m_page_top;
		m_view_top = n;
	}
	int  GetPageWidth(){ return m_pageW; }
	int  GetPageHeight(){ return m_pageH; }
	int  GetCurX(){ return m_curX; }
	int  GetCurY(){ return m_curY; }
	void SetCurX(int n){
		if(n != m_curX){
			m_curX = (n<0)? 0: (n<m_pageW-1)? n: m_pageW-1;
			m_feed_next = false;
		}
	}
	void SetCurY(int n){
		if(n != m_curY){
			m_curY = (n<0)? 0: (n<m_pageH-1)? n: m_pageH-1;
			m_feed_next = false;
		}
	}
	void MoveCurX(int n){
		m_curX+=n;
		if(m_curX<0) m_curX=0;
		else if(m_curX>=m_pageW) m_curX=m_pageW-1;
	}
	void MoveCurY(int n){
		m_curY+=n;
		if(m_curY<0) m_curY=0;
		else if(m_curY>=m_pageH) m_curY=m_pageH-1;
	}
	void MoveCurTab(int n){
		if(n>0){
			int a = 8-(m_curX % 8);
			a += 8*(n-1);
			MoveCurX(a);
		}
		else if(n<0){
			int a = m_curX % 8;
			if(a) a++;
			a += 8*(-n);
			MoveCurX(-a);
		}
	}
	void SaveCur(){ m_save_curX=m_curX; m_save_curY=m_curY; }
	void RestoreCur(){ SetCurX(m_save_curX); SetCurY(m_save_curY); }

	void Reset(bool full){
		SetRegion(0,0);
		ClearStyle(CharFlag_Styles);
		SetAddMode(false);
		if(full){
			m_curX=0;
			m_curY=0;
			ErasePage(2);
		}
	}

	void AddCharMB(WCHAR w){
		add_char(w, (CharFlag)(m_style | CharFlag_MbLead));
		add_char(w, (CharFlag)(m_style | CharFlag_MbTrail));
	}
	void AddChar(WCHAR w){
		add_char(w, m_style);
	}
	void SetAddMode(bool ins){ m_insert_mode=ins;}

	void ClearStyle(CharFlag style){
		m_style = (CharFlag)(m_style & ~style);
	}
	void SetStyle(CharFlag style){
		m_style = (CharFlag)(m_style | style);
	}
	void SetStyleFG(BYTE color){
		m_style = (CharFlag)(m_style & 0xFF00FFFF | CharFlag_FG | (color<<16));
	}
	void SetStyleBG(BYTE color){
		m_style = (CharFlag)(m_style & 0x00FFFFFF | CharFlag_BG | (int(color)<<24));
	}
	Snapshot* GetSnapshot(bool rvideo, bool viscur);
	void ClearSelection(){ m_sel_top = m_sel_btm = -1; }
	void SetSelection(int x1, int y1, int x2, int y2, int mode);
	BSTR GetSelection();
};


class Screen{
	Screen_* m_cur;
	Screen_  m_fore;
	Screen_  m_back;
	CRITICAL_SECTION m_cs;

	void _finalize(){
		DeleteCriticalSection(&m_cs);
	}
public:
	~Screen(){ _finalize();}
	Screen(): m_cur(&m_fore){
		InitializeCriticalSection(&m_cs);
	}
	void Lock(){ EnterCriticalSection(&m_cs); }
	void Unlock(){ LeaveCriticalSection(&m_cs); }
	//
	Screen_&  Current(){ return *m_cur; }
	Screen_&  Fore(){ return m_fore; }
	Screen_&  Back(){ return m_back; }
	void Set(bool b){ m_cur=(b) ? &m_back: &m_fore; }
};

}//namespace Ck
//EOF
