/*----------------------------------------------------------------------------
 * Copyright 2004-2005,2007  Kazuo Ishii <kish@wb3.so-net.ne.jp>
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
#include "screen.h"

namespace Ck{

void Screen_::_finalize(){
	if(m_buffer){
		Line* p = m_buffer;
		Line* e = m_buffer + m_maxsize;
		do{
			if(p->chars)
				free(p->chars);
			p++;
		}while(p<e);
		delete [] m_buffer;
	}
}

Screen_::Screen_()
	: m_buffer(0),
	  m_maxsize(0),
	  m_head(0),
	  m_view_top(0),
	  m_page_top(0),
	  m_pageW(0),
	  m_pageH(0),
	  m_curX(0),
	  m_curY(0),
	  m_rgn_top(0),
	  m_rgn_btm(0),
	  m_sel_top(-1),
	  m_sel_topX(0),
	  m_sel_btm(-1),
	  m_sel_btmX(0),
	  m_save_curX(0),
	  m_save_curY(0),
	  m_savelines(0),
	  m_feed_next(0),
	  m_insert_mode(0),
	  m_style((CharFlag)(0)){
	//
	try{
		m_maxsize = 1;
		m_pageW = 1;
		m_pageH = 1;
		m_buffer = new Line[m_maxsize];
		memset(m_buffer, 0, sizeof(Line) * m_maxsize);
		_resize_line(m_buffer[0]);
	}
	catch(...){
		_finalize();
		throw;
	}
}

bool Screen_::check_selection_x(int y, int l, int r){
	if(m_sel_btm >= 0){
		y += m_page_top;
		if(m_sel_top < m_sel_btm){
			if((m_sel_top < y && y < m_sel_btm) ||
			   (m_sel_top == y && r >= m_sel_topX) ||
			   (m_sel_btm == y && l < m_sel_btmX))
				return true;
		}
		else if(m_sel_btm == y){
			if((m_sel_topX <= l && l < m_sel_btmX) ||
			   (m_sel_topX <= r && r < m_sel_btmX) ||
			   (l <= m_sel_topX && m_sel_btmX <= r))
				return true;
		}
	}
	return false;
}

bool Screen_::check_selection_y(int t, int b){
	if(m_sel_btm >= 0){
		t += m_page_top;
		b += m_page_top;
		if((m_sel_top <= t && t <= m_sel_btm) ||
		   (m_sel_top <= b && b <= m_sel_btm) ||
		   (t <= m_sel_top && m_sel_btm <= b))
			return true;
	}
	return false;
}

void Screen_::clear_line(Line& p, int start, int end){
	for(; start < end; start++){
		p.chars[start].ch = '\0';
		p.chars[start].user = 0;
		p.chars[start].flags = CharFlag_Null;
	}
}

void Screen_::_resize_line(Line& p){
	if(p.maxsize < m_pageW){
		Char* tmp = (Char*) realloc(p.chars, sizeof(Char) * m_pageW);
		if(!tmp) throw std::bad_alloc();
		p.chars = tmp;
		p.maxsize = m_pageW;
	}
	clear_line(p, p.count, m_pageW);
	p.count = m_pageW;
}

void Screen_::_resize(int newsize){
	if(m_maxsize < newsize){
		newsize += 200;
		Line* tmp = new Line[newsize];
		memcpy(tmp, m_buffer+m_head, sizeof(Line) * (m_maxsize-m_head));
		memcpy(tmp+m_maxsize-m_head, m_buffer, sizeof(Line) * m_head);
		memset(tmp+m_maxsize, 0, sizeof(Line) * (newsize-m_maxsize));
		delete [] m_buffer;
		m_buffer = tmp;
		m_maxsize = newsize;
		m_head = 0;
	}
}


void Screen_::scroll_up(int top, int btm, int n){
	if(n > btm-top) n = btm-top;
	if(n<1) return;

	Line* work = new Line[n];
	for(int i=0; i < n; i++){
		work[i] = get_line(top+i);
	}
	int remain = (btm-top) - n;
	for(int i=0; i < remain; i++){
		Line& p = get_line(top+n+i);
		get_line(top+i) = p;
	}
	for(int i=0; i < n; i++){
		Line& p = get_line(btm-n+i);
		p = work[i];
		clear_line(p, 0, p.count);
	}
	delete [] work;

	if(check_selection_y(top,btm))
		ClearSelection();
}
void Screen_::scroll_down(int top, int btm, int n){
	if(n > btm-top) n = btm-top;
	if(n<1) return;

	Line* work = new Line[n];
	for(int i=0; i < n; i++){
		work[i] = get_line(btm-1-i);
	}
	int remain = (btm-top) - n;
	for(int i=1; i <= remain; i++){
		Line& p = get_line(btm-i-n);
		get_line(btm-i) = p;
	}
	for(int i=0; i < n; i++){
		Line& p = get_line(top+i);
		p = work[i];
		clear_line(p, 0, p.count);
	}
	delete [] work;

	if(check_selection_y(top,btm))
		ClearSelection();
}

void Screen_::add_char(WCHAR w, CharFlag style){
	if(m_feed_next){
		Feed();
		m_curX = 0;
	}

	if(m_insert_mode){
		InsertChar(1);
		Line& p = get_line(m_curY);
		p.chars[m_curX].ch = w;
		p.chars[m_curX].flags = style;
	}
	else{
		Line& p = get_line(m_curY);
		p.chars[m_curX].ch = w;
		p.chars[m_curX].flags = style;
		if(check_selection_x(m_curY, m_curX, m_curX))
			ClearSelection();
	}

	if(++m_curX >= m_pageW){
		m_curX--;
		m_feed_next = true;
	}
}


void Screen_::Resize(int w, int h){
	if(w < 1 || h < 1 || (w==m_pageW && h==m_pageH))
		return;

	_resize(h + m_savelines);

	if(m_pageW != w){
		m_pageW = w;
		m_feed_next = false;
		if(m_curX >= w)
			m_curX = w-1;
		if(m_sel_btm >= 0){
			if(m_sel_top < m_sel_btm || m_sel_btmX > w)
				ClearSelection();
		}
	}

	if(m_pageH < h){
		int pos = m_head + m_page_top + m_pageH;
		int n = h - m_pageH;
		int move = (m_page_top < n) ? m_page_top : n;
		m_curY += move;
		m_page_top -= move;
		m_view_top -= move;
		if(m_view_top < 0){
			m_view_top = 0;
		}
		if(m_rgn_btm){
			m_rgn_top += move;
			m_rgn_btm += move;
		}
		for(; move < n; move++){
			if(pos >= m_maxsize)
				pos -= m_maxsize;
			Line* p = m_buffer + (pos++);
			p->count = 0;
		}
		m_pageH = h;
	}
	else if(m_pageH > h){
		int n = m_pageH - h;
		int move = (n) - (m_pageH - m_curY -1);
		if(move > 0){
			m_curY -= move;
			m_page_top += move;
			m_view_top += move;
			if(m_rgn_btm){
				m_rgn_top -= move;
				m_rgn_btm -= move;
				if(m_rgn_top < 0)
					m_rgn_top = m_rgn_btm = 0;
			}
			n = m_page_top - m_savelines;
			if(n > 0){
				m_head += n;
				if(m_head >= m_maxsize)
					m_head -= m_maxsize;
				m_page_top -= n;
				m_view_top -= n;
				if(m_view_top < 0)
					m_view_top = 0;
			}
		}
		if(m_rgn_btm){
			if(m_rgn_btm >= h-1)
				m_rgn_top = m_rgn_btm = 0;
		}
		if(m_sel_btm >= 0){
			if(m_sel_btm >= m_page_top+h)
				ClearSelection();
		}
		m_pageH = h;
	}

	int pos = m_head + m_page_top + 0;
	int n = m_pageH;
	do{
		if(pos >= m_maxsize)
			pos -= m_maxsize;
		_resize_line( m_buffer[pos++] );
	}while(--n);
}


void Screen_::ErasePage(int mode){
	if(mode==0){
		{
			Line& p = get_line(m_curY);
			clear_line(p, m_curX, p.count);
		}
		for(int i=m_curY+1; i < m_pageH; i++){
			Line& p = get_line(i);
			clear_line(p, 0, p.count);
		}
		if(check_selection_y(m_curY+1, m_pageH-1) || check_selection_x(m_curY, m_curX,m_pageW-1))
			ClearSelection();
	}
	else if(mode==1){
		{
			Line& p = get_line(m_curY);
			clear_line(p, 0, m_curX+1);
		}
		for(int i=0; i < m_curY; i++){
			Line& p = get_line(i);
			clear_line(p, 0, p.count);
		}
		if(check_selection_y(0, m_curY-1) || check_selection_x(m_curY, 0, m_curX))
			ClearSelection();
	}
	else if(mode==2){
		for(int i=0 ; i < m_pageH; i++){
			Line& p = get_line(i);
			clear_line(p, 0, p.count);
		}
		if(check_selection_y(0, m_pageH-1))
			ClearSelection();
	}
	else{
		return;
	}
	m_feed_next = false;
}

void Screen_::EraseLine(int mode){
	if(mode==0){
		Line& p = get_line(m_curY);
		clear_line(p, m_curX, p.count);
		if(check_selection_x(m_curY, m_curX, m_pageW-1))
			ClearSelection();
	}
	else if(mode==1){
		Line& p = get_line(m_curY);
		clear_line(p, 0, m_curX+1);
		if(check_selection_x(m_curY, 0, m_curX))
			ClearSelection();
	}
	else if(mode==2){
		Line& p = get_line(m_curY);
		clear_line(p, 0, p.count);
		if(check_selection_y(m_curY, m_curY))
			ClearSelection();
	}
	else{
		return;
	}
	m_feed_next = false;
}

void Screen_::SetRegion(int top, int btm){
	if(top < 0 ||
	   top >= btm ||
	   top >= m_pageH-1 ||
	   (top==0 && btm >= m_pageH-1)){
		m_rgn_top = m_rgn_btm = 0;
	}
	else{
		if(btm >= m_pageH)
			btm = m_pageH-1;
		m_rgn_top = top;
		m_rgn_btm = btm;
	}
}

void Screen_::ScrollPage(int n){
	if(n==0) return;
	int top,btm;
	if(m_rgn_btm){
		top = m_rgn_top;
		btm = m_rgn_btm+1;
	}
	else{
		top = 0;
		btm = m_pageH;
	}
	if(n > 0) scroll_up(top,btm, n);
	else      scroll_down(top,btm, -n);
	m_feed_next = false;
}
void Screen_::InsertLine(int n){
	int btm;
	if(m_rgn_btm){
		if(m_curY <= m_rgn_btm)
			btm = m_rgn_btm+1;
		else
			return;
	}
	else{
		btm = m_pageH;
	}
	scroll_down(m_curY, btm, (n<1)? 1: n);
	m_feed_next = false;
}
void Screen_::DeleteLine(int n){
	int btm;
	if(m_rgn_btm){
		if(m_curY <= m_rgn_btm)
			btm = m_rgn_btm+1;
		else
			return;
	}
	else{
		btm = m_pageH;
	}
	scroll_up(m_curY, btm, (n<1)? 1: n);
	m_feed_next = false;
}

void Screen_::Feed(){
	if(m_rgn_btm && m_rgn_btm == m_curY){
		scroll_up(m_rgn_top, m_rgn_btm+1, 1);
	}
	else if(m_rgn_btm && m_rgn_btm <= m_curY && m_curY == m_pageH-1){
	}
	else if(m_curY < m_pageH-1){
		m_curY++;
	}
	else{
		if(m_page_top >= m_savelines){
			if(++m_head >= m_maxsize)
				m_head -= m_maxsize;
			if(m_sel_btm >= 0){
				--m_sel_btm;
				if(--m_sel_top < 0)
					ClearSelection();
			}
		}
		else{
			m_page_top++;
			m_view_top++;
		}

		Line& p = get_line(m_pageH-1);
		p.count = 0;
		_resize_line(p);
	}
	m_feed_next = false;
}

void Screen_::FeedRev(){
	if(!m_rgn_btm && m_curY == 0){
		scroll_down(0, m_pageH, 1);
	}
	else if(m_rgn_btm && m_curY == m_rgn_top){
		scroll_down(m_rgn_top, m_rgn_btm+1, 1);
	}
	else if(m_curY > 0){
		m_curY--;
	}
	m_feed_next = false;
}

void Screen_::InsertChar(int n){
	if(n<1) n=1;
	if(n > m_pageW - m_curX)
		n = m_pageW - m_curX;
	Line& p = get_line(m_curY);
	for(int i=m_pageW-1; i >= m_curX+n; i--)
		p.chars[i] = p.chars[i-n];
	clear_line(p, m_curX, m_curX+n);

	if(check_selection_x(m_curY, m_curX, m_pageW-1))
		ClearSelection();
}
void Screen_::DeleteChar(int n){
	if(n<1) n=1;
	if(n > m_pageW - m_curX)
		n = m_pageW - m_curX;
	Line& p = get_line(m_curY);
	for(int i=m_curX; i < m_pageW-n; i++)
		p.chars[i] = p.chars[i+n];
	clear_line(p, m_pageW-n, m_pageW);

	if(check_selection_x(m_curY, m_curX, m_pageW-1))
		ClearSelection();
}
void Screen_::EraseChar(int n){
	if(n<1) n=1;
	if(n > m_pageW - m_curX)
		n = m_pageW - m_curX;
	Line& p = get_line(m_curY);
	clear_line(p, m_curX, m_curX+n);

	if(check_selection_x(m_curY, m_curX, m_curX+n-1))
		ClearSelection();
}

//

Snapshot*  Screen_::GetSnapshot(bool rvideo, bool viscur){

	Snapshot* ss = (Snapshot*) new BYTE[ sizeof(Snapshot) + (sizeof(Char) * m_pageW * m_pageH) ];
	ss->width  = m_pageW;
	ss->height = m_pageH;
	ss->view   = m_view_top;
	ss->nlines = m_page_top+m_pageH;
	ss->cursor_x = -1;
	ss->cursor_y = -1;

	int viewy = m_view_top;
	int maxy = viewy + m_pageH;
	int pos = m_head + viewy;
	int i = 0;

	CharFlag inv = (CharFlag)(0);
	if(rvideo)
		inv = (CharFlag)(inv | CharFlag_Invert);
	if(m_sel_btm >= 0 && m_sel_top < viewy && viewy <= m_sel_btm)
		inv = (CharFlag)(inv | CharFlag_Select);

	for(; viewy < maxy ; viewy++, pos++){
		if(pos >= m_maxsize)
			pos -= m_maxsize;
		Line& line = m_buffer[pos];
		for(int x=0; x < m_pageW; x++, i++){
			if(inv & CharFlag_Select){
				if(m_sel_btm == viewy && m_sel_btmX==x)
					inv = (CharFlag)(inv & ~CharFlag_Select);
			}
			else{
				if(m_sel_top == viewy && m_sel_topX==x)
					inv = (CharFlag)(inv | CharFlag_Select);
			}
			if(x < line.count){
				ss->chars[i] = line.chars[x];
				ss->chars[i].flags = (CharFlag)(ss->chars[i].flags ^ inv);
			}
			else{
				ss->chars[i].ch = '\0';
				ss->chars[i].user = 0;
				ss->chars[i].flags = (CharFlag)(CharFlag_Null ^ inv);
			}
		}
		if(viewy == m_sel_btm && m_pageW==m_sel_btmX){
			inv = (CharFlag)(inv & ~CharFlag_Select);
		}
	}

	if(viscur){
		i = m_page_top + m_curY;
		if(m_view_top <= i && i < maxy){
			ss->cursor_x = m_curX;
			ss->cursor_y = (i-m_view_top);
			i = (i-m_view_top) * m_pageW + m_curX;
			ss->chars[i].flags = (CharFlag)(ss->chars[i].flags | CharFlag_Cursor);
		}
	}

	return ss;
}

void Screen_::SetSelection(int x1, int y1, int x2, int y2, int mode){
	if(mode < 0 || 2 < mode){
		m_sel_top = m_sel_btm = -1;
		return;
	}

	int maxY = GetNumLines()-1;
	if(y1<0)y1=0; else if(y1>maxY) y1=maxY;
	if(y2<0)y2=0; else if(y2>maxY) y2=maxY;
	if(x1<0)x1=0;
	if(x2<0)x2=0;
	if(y1 < y2 || (y1==y2 && x1<x2)){
		m_sel_top  = y1;
		m_sel_topX = x1;
		m_sel_btm  = y2;
		m_sel_btmX = x2;
	}
	else{
		m_sel_top  = y2;
		m_sel_topX = x2;
		m_sel_btm  = y1;
		m_sel_btmX = x1;
	}
	if(m_sel_topX >= m_pageW) m_sel_topX = m_pageW-1;
	if(m_sel_btmX >  m_pageW) m_sel_btmX = m_pageW;

	if(mode==2){ //expand line
		m_sel_topX=0;
		m_sel_btmX=m_pageW;
	}
	else if(mode==1){//expand word
		static const wchar_t BREAK_CHARS[] = L"\"&'()*,./:;<=>@[\\]^`{}~\x3000\x3001\x3002\x300C\x300D\x3010\x3011";
		Line& p1 = get_line_glb(m_sel_top);
		if(m_sel_topX < p1.count){
			while(m_sel_topX > 0){
				Char& c = p1.chars[m_sel_topX-1];
				if(c.ch <= ' ' || wcschr(BREAK_CHARS, c.ch)!=0)
					break;
				m_sel_topX--;
			}
		}
		Line& p2 = get_line_glb(m_sel_btm);
		if(m_sel_btmX < p2.count){
			while(m_sel_btmX < m_pageW){
				Char& c = p2.chars[m_sel_btmX];
				if(c.ch <= ' ' || wcschr(BREAK_CHARS, c.ch)!=0)
					break;
				m_sel_btmX++;
			}
		}
	}
	else{//expand char
		Line& p1 = get_line_glb(m_sel_top);
		if(m_sel_topX < p1.count){
			if(m_sel_topX > 0){
				Char& c = p1.chars[m_sel_topX-1];
				if(c.flags & CharFlag_MbTrail)
					m_sel_topX--;
			}
		}
		Line& p2 = get_line_glb(m_sel_btm);
		if(m_sel_btmX < p2.count){
			if(m_sel_btmX < m_pageW){
				Char& c = p2.chars[m_sel_btmX];
				if(c.flags & CharFlag_MbTrail)
					m_sel_btmX++;
			}
		}
	}

	if(m_sel_top == m_sel_btm && m_sel_topX >= m_sel_btmX){
		m_sel_top = m_sel_btm = -1;
	}
}


static void copy_chars(WCHAR*& dst, Line& p, int start, int max, bool addLF){
	if(max < start || max > p.count)
		max = p.count;
	while( max>start && p.chars[max-1].flags & CharFlag_Null )
		max--;
	for(; start < max ; start++){
		if(p.chars[start].flags & CharFlag_MbTrail){
		}
		else if(p.chars[start].ch >= ' '){
			*dst++ = p.chars[start].ch;
		}
		else{
			*dst++ = ' ';
		}
	}
	if(addLF && max < p.count){
		*dst++ = '\r';
		*dst++ = '\n';
	}
}

BSTR Screen_::GetSelection(){
	BSTR outbuf;
	WCHAR* ws;
	if(m_sel_btm >= 0){
		if(m_sel_top < m_sel_btm){
			int nb = 1;
			int i = m_sel_top;
			for(; i<=m_sel_btm; i++)
				nb += get_line_glb(i).count + 2;
			i = m_sel_top;
			//
			outbuf = SysAllocStringLen(0,nb);
			ws = outbuf;
			if(outbuf){
				copy_chars(ws, get_line_glb(i), m_sel_topX, -1, true);
				for(i++; i < m_sel_btm; i++)
					copy_chars(ws, get_line_glb(i), 0, -1, true);
				copy_chars(ws, get_line_glb(i), 0, m_sel_btmX, false);
			}
		}
		else{
			Line& line = get_line_glb(m_sel_top);
			outbuf = SysAllocStringLen(0, line.count+1);
			ws = outbuf;
			if(outbuf){
				copy_chars(ws, line, m_sel_topX, m_sel_btmX, false);
			}
		}
		//
		*ws = '\0';
		((UINT*)outbuf)[-1] = (UINT)((BYTE*)ws - (BYTE*)outbuf);
	}
	else{
		outbuf = SysAllocStringLen(L"", 0);
	}
	return outbuf;
}

}//namespace Ck
//EOF
