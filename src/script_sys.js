var Keys ={
  ShiftL:0x01000000,
  ShiftR:0x02000000,
  Shift :0x01000000 | 0x02000000,
  CtrlL :0x04000000,
  CtrlR :0x08000000,
  Ctrl  :0x04000000 | 0x08000000,
  AltL  :0x10000000,
  AltR  :0x20000000,
  Alt   :0x10000000 | 0x20000000,
  None:0,
  Back:8,
  Tab:9,
  Clear:12,
  Return:13,
  Pause:19,
  KanaMode:21,
  JunjaMode:23,
  FinalMode:24,
  HanjaMode:25,
  Escape:27,
  IMEConvert:28,
  IMENonconvert:29,
  IMEAceept:30,
  IMEModeChange:31,
  Space:32,
  PageUp:33,
  PageDown:34,
  End:35,
  Home:36,
  Left:37,
  Up:38,
  Right:39,
  Down:40,
  Select:41,
  Print:42,
  Execute:43,
  PrintScreen:44,
  Insert:45,
  Delete:46,
  Help:47,
  D0:48,
  D1:49,
  D2:50,
  D3:51,
  D4:52,
  D5:53,
  D6:54,
  D7:55,
  D8:56,
  D9:57,
  A:65,
  B:66,
  C:67,
  D:68,
  E:69,
  F:70,
  G:71,
  H:72,
  I:73,
  J:74,
  K:75,
  L:76,
  M:77,
  N:78,
  O:79,
  P:80,
  Q:81,
  R:82,
  S:83,
  T:84,
  U:85,
  V:86,
  W:87,
  X:88,
  Y:89,
  Z:90,
  LWin:91,
  RWin:92,
  Apps:93,
  NumPad0:96,
  NumPad1:97,
  NumPad2:98,
  NumPad3:99,
  NumPad4:100,
  NumPad5:101,
  NumPad6:102,
  NumPad7:103,
  NumPad8:104,
  NumPad9:105,
  Multiply:106,
  Add:107,
  Separator:108,
  Subtract:109,
  Decimal:110,
  Divide:111,
  F1:112,
  F2:113,
  F3:114,
  F4:115,
  F5:116,
  F6:117,
  F7:118,
  F8:119,
  F9:120,
  F10:121,
  F11:122,
  F12:123,
  F13:124,
  F14:125,
  F15:126,
  F16:127,
  F17:128,
  F18:129,
  F19:130,
  F20:131,
  F21:132,
  F22:133,
  F23:134,
  F24:135,
  Scroll:145,
  BrowserBack:166,
  BrowserForward:167,
  BrowserRefresh:168,
  BrowserStop:169,
  BrowserSearch:170,
  BrowserFavorites:171,
  BrowserHome:172,
  VolumeMute:173,
  VolumeDown:174,
  VolumeUp:175,
  MediaNextTrack:176,
  MediaPreviousTrack:177,
  MediaStop:178,
  MediaPlayPause:179,
  LaunchMail:180,
  SelectMedia:181,
  LaunchApplication1:182,
  LaunchApplication2:183,
  Oem1:186,
  Oemplus:187,
  Oemcomma:188,
  OemMinus:189,
  OemPeriod:190,
  OemQuestion:191,
  Oemtilde:192,
  OemOpenBrackets:219,
  Oem5:220,
  Oem6:221,
  Oem7:222,
  Oem8:223,
  OemBackslash:226,
  ProcessKey:229,
  Packet:231,
  Attn:246,
  Crsel:247,
  Exsel:248,
  EraseEof:249,
  Play:250,
  Zoom:251,
  OemClear:254
};

var Encoding ={
  EUCJP  :0x01,
  SJIS  :0x02,
  UTF8  :0x04
};

var Priv ={
  None                    :0x00000000,
  Col132                  :0x00000001,
  Allow132                :0x00000002,
  RVideo                  :0x00000004,
  RelOrg                  :0x00000008,
  Screen                  :0x00000010,
  AutoWrap                :0x00000020,
  AplCUR                  :0x00000040,
  AplKP                   :0x00000080,
  Backspace               :0x00000100,
  ShiftKey                :0x00000200,
  VisibleCur              :0x00000400,

  MouseModeMask           :0x00003800,
  MouseMode_None          :0x00000000,
  MouseMode_Normal        :0x00000800,
  MouseMode_Highlight     :0x00001000,
  MouseMode_ButtonEvent   :0x00001800,
  MouseMode_AnyEvent      :0x00002000,

  MouseProtocolMask       :0x0000c000,
  MouseProtocol_X11       :0x00000000,
  MouseProtocol_X10       :0x00004000,
  MouseProtocol_Urxvt     :0x00008000,
  MouseProtocol_Sgr       :0x0000c000,

  ScrlTtyOut              :0x00020000,
  ScrlTtyKey              :0x00040000,
  SmoothScrl              :0x00080000,
  VT52                    :0x00100000,
  UseBell                 :0x20000000,
  CjkWidth                :0x40000000
};

var WinTransp ={
  None  :0,
  Transp  :1,
  Grass  :2,
  GrassNoEdge :3
};

var WinZOrder ={
  Normal  :0,
  Top  :1,
  Bottom  :2
};

var MouseCmd ={
  None  :0,
  Select  :1,
  Paste  :2,
  Menu  :3
};

var Place ={
  Scale  :0,
  Zoom  :1,
  Repeat  :2,
  NoRepeat:3
};

var Align ={
  Near  :0,
  Center  :1,
  Far  :2
};

function CONFIG(){
  this.tty={
    execute_command  :"/bin/bash --login -i",
    title    :"ck",
    savelines  :1000,
    input_encoding  :Encoding.SJIS,
    display_encoding:Encoding.SJIS | Encoding.EUCJP | Encoding.UTF8,
    scroll_key  :1,
    scroll_output  :0,
    bs_as_del  :0,
    use_bell  :0,
    cjk_width  :1
  };
  this.accelkey={
    new_shell  :Keys.ShiftL | Keys.CtrlL | Keys.N,
    new_window  :Keys.ShiftL | Keys.CtrlL | Keys.M,
    open_window  :Keys.ShiftL | Keys.CtrlL | Keys.O,
    close_window  :Keys.ShiftL | Keys.CtrlL | Keys.W,
    next_shell  :Keys.CtrlL  | Keys.Tab,
    prev_shell  :Keys.ShiftL | Keys.CtrlL | Keys.Tab,
    paste    :Keys.ShiftL | Keys.Insert,
    popup_menu  :Keys.ShiftL | Keys.F10,
    popup_sys_menu  :Keys.AltR   | Keys.Space,
    scroll_page_up  :Keys.ShiftL | Keys.PageUp,
    scroll_page_down:Keys.ShiftL | Keys.PageDown,
    scroll_line_up  :Keys.None,
    scroll_line_down:Keys.None,
    scroll_top  :Keys.ShiftL | Keys.Home,
    scroll_bottom  :Keys.ShiftL | Keys.End
  };
  this.window={
    position_x  :null,
    position_y  :null,
    cols    :80,
    rows    :24,
    scrollbar_show  :1,
    scrollbar_right  :1,
    blink_cursor  :1,
    transparent  :WinTransp.None,
    zorder    :WinZOrder.Normal,
    linespace  :0,
    border_left  :1,
    border_top  :1,
    border_right  :1,
    border_bottom  :1,
    mouse_left  :MouseCmd.Select,
    mouse_middle  :MouseCmd.Paste,
    mouse_right  :MouseCmd.Menu,
    font_name  :"MS Gothic",
    font_size  :12,
    background_file  :"",
    background_repeat_x  :Place.NoRepeat,
    background_repeat_y  :Place.NoRepeat,
    background_align_x  :Align.Center,
    background_align_y  :Align.Center,
    alpha_text_border  :0x00,
    alpha_back_colorN  :0xDD,
    color_foreground  :0x000000,
    color_background  :0xFFFFFFFF,
    color_selection    :0x660000FF,
    color_cursor    :0x00AA00,
    color_imecursor    :0xAA0000
  };
}
CONFIG.prototype ={
  copyTo_pty : function(pty){
    var n = pty.PrivMode;
    n = (this.tty.scroll_key) ? (n | Priv.ScrlTtyKey) : (n & ~Priv.ScrlTtyKey);
    n = (this.tty.scroll_output) ? (n | Priv.ScrlTtyOut) : (n & ~Priv.ScrlTtyOut);
    n = (this.tty.bs_as_del) ? (n | Priv.Backspace) : (n & ~Priv.Backspace);
    n = (this.tty.use_bell)  ? (n | Priv.UseBell)  : (n & ~Priv.UseBell);
    n = (this.tty.cjk_width) ? (n | Priv.CjkWidth) : (n & ~Priv.CjkWidth);
    pty.PrivMode = n;
    pty.InputEncoding = this.tty.input_encoding;
    pty.DisplayEncoding = this.tty.display_encoding;
    pty.Savelines = this.tty.savelines;
    pty.Title = this.tty.title;
  },
  copyFrom_pty : function(pty){
    var n = pty.PrivMode;
    this.tty.scroll_key = (n & Priv.ScrlTtyKey) ? 1 : 0;
    this.tty.scroll_output = (n & Priv.ScrlTtyKey) ? 1 : 0;
    this.tty.bs_as_del = (n & Priv.Backspace) ? 1 : 0;
    this.tty.use_bell  = (n & Priv.UseBell)  ? 1 : 0;
    this.tty.cjk_width = (n & Priv.CjkWidth) ? 1 : 0;
    this.tty.input_encoding = pty.InputEncoding;
    this.tty.display_encoding = pty.DisplayEncoding;
    this.tty.savelines = pty.Savelines;
    this.tty.workdir = pty.CurrentDirectory;
  },
  copyTo_window : function(window){
    for(var i=0; i <= 15; i++){
      var v = this.window["color_color"+i];
      if(v != null) window.Color(i) = v;
    }
    window.Color(256) = this.window.color_foreground;
    window.Color(257) = this.window.color_background;
    window.Color(258) = this.window.color_selection;
    window.Color(259) = this.window.color_cursor;
    window.Color(260) = this.window.color_imecursor;
    window.ColorAlpha(1) = this.window.alpha_text_border;
    window.ColorAlpha(0) = this.window.alpha_back_colorN;
    window.FontName = this.window.font_name;
    window.FontSize = this.window.font_size;
    window.BackgroundPlace =
      (this.window.background_repeat_y << 24) |
      (this.window.background_align_y  << 16) |
      (this.window.background_repeat_x << 8 ) |
      (this.window.background_align_x  << 0 );
    if(this.window.background_file != null &&
       this.window.background_file != ""){
      app.trace("[file] \"" + this.window.background_file + "\"\n");
      window.BackgroundImage = this.window.background_file;
    }
    window.Border =
      (this.window.border_left   << 24) |
      (this.window.border_top    << 16) |
      (this.window.border_right  << 8) |
      (this.window.border_bottom << 0);
    window.LineSpace = this.window.linespace;
    window.ZOrder = this.window.zorder;
    window.Transp = this.window.transparent;
    window.Scrollbar = (this.window.scrollbar_show ? 1 : 0) | (this.window.scrollbar_right ? 0 : 2);
    window.BlinkCursor = this.window.blink_cursor;
    window.MouseLBtn = this.window.mouse_left;
    window.MouseMBtn = this.window.mouse_middle;
    window.MouseRBtn = this.window.mouse_right;
    window.Resize(this.window.cols, this.window.rows);
    if(this.window.position_x != null)
      window.Move(this.window.position_x, this.window.position_y);
  },
  copyFrom_window : function(window){
    for(var i=0; i <= 15; i++)
      this.window["color_color"+i] = window.Color(i);
    this.window.color_foreground = window.Color(256);
    this.window.color_background = window.Color(257);
    this.window.color_selection = window.Color(258);
    this.window.color_cursor = window.Color(259);
    this.window.color_imecursor = window.Color(260);
    this.window.alpha_text_border = window.ColorAlpha(1);
    this.window.alpha_back_colorN = window.ColorAlpha(0);
    this.window.font_name = window.FontName;
    this.window.font_size = window.FontSize;
    var n = window.BackgroundPlace;
    this.window.background_repeat_y = (n>>24)&0xff;
    this.window.background_align_y  = (n>>16)&0xff;
    this.window.background_repeat_x = (n>>8 )&0xff;
    this.window.background_align_x  = (n>>0 )&0xff;
    this.window.background_file = window.BackgroundImage;
    n = window.Border;
    this.window.border_left   = (n>>24)&0xff;
    this.window.border_top    = (n>>16)&0xff;
    this.window.border_right  = (n>>8 )&0xff;
    this.window.border_bottom = (n>>0 )&0xff;
    this.window.linespace = window.LineSpace;
    this.window.zorder = window.ZOrder;
    this.window.transparent = window.Transp;
    n = window.Scrollbar;
    this.window.scrollbar_show  = (n&1) ? 1 : 0;
    this.window.scrollbar_right = (n&2) ? 0 : 1;
    this.window.blink_cursor = window.BlinkCursor;
    this.window.mouse_left   = window.MouseLBtn;
    this.window.mouse_middle = window.MouseMBtn;
    this.window.mouse_right  = window.MouseRBtn;
  },
  copyFrom_args : function(opt){
    if(opt.fg != null) this.window.color_foreground = opt.fg;
    if(opt.bg != null) this.window.color_background = opt.bg;
    if(opt.cr != null) this.window.color_cursor = opt.cr;
    if(opt.bgbmp != null) this.window.background_file = opt.bgbmp;
    if(opt.fn != null) this.window.font_name = opt.fn;
    if(opt.fs != null) this.window.font_size = opt.fs;
    if(opt.lsp != null) this.window.linespace = opt.lsp;
    if(opt.sb != null) this.window.scrollbar_show = opt.sb ? 1 : 0;
    if(opt.posx != null) this.window.position_x = opt.posx;
    if(opt.posy != null) this.window.position_y = opt.posy;
    if(opt.cols != null) this.window.cols = opt.cols;
    if(opt.rows != null) this.window.rows = opt.rows;
    if(opt.si != null) this.tty.scroll_output = opt.si;
    if(opt.sk != null) this.tty.scroll_key = opt.sk;
    if(opt.sl != null) this.tty.savelines = opt.sl;
    if(opt.bs != null) this.tty.bs_as_del = opt.bs;
    if(opt.cjk != null)this.tty.cjk_width = opt.cjk;
    if(opt.km != null) this.tty.input_encoding = opt.km;
    if(opt.md != null) this.tty.display_encoding = opt.md;
    if(opt.cmd != null)this.tty.execute_command = opt.cmd;
    if(opt.title != null)this.tty.title = opt.title;
    if(opt.workdir != null)this.tty.workdir = opt.workdir;
  },
  copyTo : function(dst){
    for(pn in this.tty)      dst.tty[pn] = this.tty[pn];
    for(pn in this.window)   dst.window[pn] = this.window[pn];
    for(pn in this.accelkey) dst.accelkey[pn] = this.accelkey[pn];
  },
  clone : function(){
    var n = new CONFIG();
    this.copyTo(n);
    return n;
  }
};

function ARGS(args){
  var nb = args.length;
  for(var i=1; i < nb; i++){
    var s = args[i];
    if(s=="-share"){}
    else if(s=="-fg")   { if(++i < nb) this.fg = parseInt(args[i], 16);}
    else if(s=="-bg")   { if(++i < nb) this.bg = parseInt(args[i], 16);}
    else if(s=="-cr")   { if(++i < nb) this.cr = parseInt(args[i], 16);}
    else if(s=="-bgbmp"){ if(++i < nb) this.bgbmp = args[i];}
    else if(s=="-fn")   { if(++i < nb) this.fn = args[i];}
    else if(s=="-fs")   { if(++i < nb) this.fs = parseInt(args[i]);}
    else if(s=="-lsp")  { if(++i < nb) this.lsp = parseInt(args[i]);}
    else if(s=="-sl")   { if(++i < nb) this.sl = parseInt(args[i]);}
    else if(s=="-si"  || s=="+si"){ this.si = (s.charAt(0)=='-') ? 1 : 0;}
    else if(s=="-sk"  || s=="+sk"){ this.sk = (s.charAt(0)=='-') ? 1 : 0;}
    else if(s=="-sb"  || s=="+sb"){ this.sb = (s.charAt(0)=='-') ? 1 : 0;}
    else if(s=="-bs"  || s=="+bs"){ this.bs = (s.charAt(0)=='-') ? 1 : 0;}
    else if(s=="-cjk" || s=="+cjk"){ this.cjk = (s.charAt(0)=='-') ? 1 : 0;}
    else if(s=="-km")   { if(++i < nb) this.km = Helper.get_encoding(args[i]);}
    else if(s=="-md")   { if(++i < nb) this.md = Helper.get_encoding(args[i]);}
    else if(s=="-title"){ if(++i < nb) this.title = args[i];}
    else if(s=="-f")    { if(++i < nb) this.script = args[i];}
    else if(s=="-dir")  { if(++i < nb) this.workdir = args[i];}
    else if(s=="-g"){
      if(++i < nb){
        if(args[i].match(/^(\d+)x(\d+)([\+\-])(\d+)([\+\-])(\d+)/)){
          s = parseInt(RegExp.$4);
          this.posx = (RegExp.$3 == '-') ? -s-1 : s;
          s = parseInt(RegExp.$6);
          this.posy = (RegExp.$5 == '-') ? -s-1 : s;
          this.cols = parseInt(RegExp.$1);
          this.rows = parseInt(RegExp.$2);
        }
        else if(args[i].match(/^(\d+)x(\d+)/)){
          this.cols = parseInt(RegExp.$1);
          this.rows = parseInt(RegExp.$2);
        }
      }
    }
    else if(s=="-e"){
      if(++i < nb){
        this.cmd = "";
        while(i<nb) this.cmd += "\"" + args[i++] + "\" ";
      }
    }
  }
}

function ACCEL(key,func,menu){
  this.key = key;
  this.exec = func;
  this.menu = menu;
}
ACCEL.prototype ={
  chkmod : function(k, mask){
    var b = this.key & mask;
    if(b){
      if(!(k & b))
        return false;
    }else{
      if(k & mask)
        return false;
    }
    return true;
  },
  match : function(k){
    if((this.key & 0x00FFFFFF)==(k & 0x00FFFFFF) &&
       this.chkmod(k, Keys.Ctrl ) &&
       this.chkmod(k, Keys.Shift) &&
       this.chkmod(k, Keys.Alt  ))
      return true;
    return false;
  }
};


function LIST(){
  this.array = [];
}
LIST.prototype ={
  count : function(){
    return this.array.length;
  },
  item : function(i){
    return this.array[i];
  },
  add : function(p){
    this.array.push(p);
  },
  remove : function(p){
    var i=0, nb=this.array.length;
    for(; i<nb; i++){
      if(p == this.array[i]){
        for(; i<nb-1; i++)
          this.array[i] = this.array[i+1];
        this.array.pop();
        p.Dispose();//
        return;
      }
    }
  },
  clear : function(){
    var i=0, nb=this.array.length;
    for(; nb>0; nb--){
      this.array.pop().Dispose();//
    }
  }
};



//------------------
var HomeDir = app.Env("HOME"); //$HOME is set with cygwin_dll_init()

var Config = new CONFIG();

var PtyList = new LIST();
var WndList = new LIST();

var AccelKeys = [];

AccelKeys.add = function(key, cmd, menu){
  if(cmd != null && key != null && (key != Keys.None || menu != null))
    this.push(new ACCEL(key, cmd, menu));
}

//------------------

var Helper ={
  set_input_encoding : function(window, bit){
    var p = window.Pty;
    if(p) p.InputEncoding = bit;
  },
  toggle_display_encoding : function(window, bit){
    var p = window.Pty;
    if(p){
      var e = p.DisplayEncoding ^ bit;
      if(e)
        p.DisplayEncoding = e;
    }
  },
  toggle_priv_mode : function(window, bit){
    var p = window.Pty;
    if(p) p.PrivMode = p.PrivMode ^ bit;
  },
  scroll_absolute : function(window, n){
    var p = window.Pty;
    if(p) p.ViewPos = n;
  },
  scroll_relative : function(window, page, line){
    var p = window.Pty;
    if(p){
      n = (p.ViewPos) + (p.PageHeight * page) + (line);
      if(n<0) n=0;
      p.ViewPos = n;
    }
  },
  escape_path : function(path){
    var checklist = " #$%&*()[]{};\"\\!`'<>?";
    for(var i=0; i < checklist.length; i++){
      var c = checklist.charAt(i);
      if(path.indexOf(c) >= 0){
        path = path.replace(/\"/g, "\\\"");
        return "\"" + path + "\"";
      }
    }
    return path;
  },
  get_encoding : function(str){
    var ar = str.replace(/\s/, '').split(',');
    var n = 0;
    for(var i=0; i < ar.length; i++){
      if(ar[i]=="sjis" || ar[i]=="shiftjis") n |= Encoding.SJIS;
      else if(ar[i]=="eucj" || ar[i]=="eucjp") n |= Encoding.EUCJP;
      else if(ar[i]=="utf8" || ar[i]=="utf-8") n |= Encoding.UTF8;
    }
    return (n != 0) ? n : null;
  },
  get_active_window : function(){
    var active = app.ActiveWindow;
    var nbwnd = WndList.count();
    for(var i=0; i < nbwnd; i++){
      var w = WndList.item(i);
      if(w.Handle == active)
        return w;
    }
    return null;
  },
  create_shell : function(cfg){
    if(cfg.tty.workdir != null)
      app.CurrentDirectory = cfg.tty.workdir;

    var n = app.NewPty( cfg.tty.execute_command );
    PtyList.add(n);

    if(cfg.tty.workdir != null)
      app.CurrentDirectory = HomeDir;

    cfg.copyTo_pty(n);

    for(var i=0; i < WndList.count(); i++){
      Events.wnd_on_title_init(WndList.item(i));
    }
    return n;
  },
  create_window : function(cfg){
    var n = app.NewWnd();
    WndList.add(n);

    cfg.copyTo_window(n);
    return n;
  }
};

var Commands ={
  wnd_next_shell : function(window){
    var nbpty = PtyList.count();
    if(nbpty>1){
      var pty = window.Pty;
      for(var i=0; i < nbpty; i++){
        if(PtyList.item(i) == pty){
          if(++i >= nbpty) i=0;
          window.Pty = PtyList.item(i);
          break;
        }
      }
    }
    return true;
  },
  wnd_prev_shell : function(window){
    var nbpty = PtyList.count();
    if(nbpty>1){
      var pty = window.Pty;
      for(var i=0; i < nbpty; i++){
        if(PtyList.item(i) == pty){
          if(--i < 0) i=nbpty-1;
          window.Pty = PtyList.item(i);
          break;
        }
      }
    }
    return true;
  },
  wnd_new_shell : function(window){
    var cfg = Config.clone();
    if(window != null){
      var pty = window.Pty;
      if(pty != null){
        cfg.copyFrom_pty(pty);
      }
    }
    window.Pty = Helper.create_shell(cfg);
    return true;
  },
  wnd_new_window : function(window){
    var cfg = Config.clone();
    if(window != null){
      var pty = window.Pty;
      if(pty != null){
        cfg.copyFrom_pty(pty);
      }
    }
    cfg.copyFrom_window(window);
    var p = Helper.create_shell(cfg);
    var w = Helper.create_window(cfg);
    w.Pty = p;
    w.Show();
    return true;
  },
  wnd_open_window : function(window){
    var cfg = Config.clone();
    cfg.copyFrom_window(window);
    var w = Helper.create_window(cfg);
    w.Pty = PtyList.item(0);
    w.Show();
    return true;
  },
  wnd_close_window : function(window){
    window.Close();
    return true;
  },
  tty_paste : function(window){
    var p = window.Pty;
    if(p) p.PutString(app.Clipboard);
    return true;
  },
  tty_reset : function(window){
    var p = window.Pty;
    if(p){
      p.Reset(true);
      window.UpdateScreen();
    }
    return true;
  },
  tty_scroll_page_up : function(window)    { Helper.scroll_relative(window, -1, 0); return true;},
  tty_scroll_page_down : function(window)    { Helper.scroll_relative(window, +1, 0); return true;},
  tty_scroll_line_up : function(window)    { Helper.scroll_relative(window, 0, -3); return true;},
  tty_scroll_line_down : function(window)    { Helper.scroll_relative(window, 0, +3); return true;},
  tty_scroll_top : function(window)    { Helper.scroll_absolute(window, 0); return true;},
  tty_scroll_bottom : function(window)    { Helper.scroll_absolute(window, -1); return true;},
  tty_enc_set_input_sjis : function(window)  { Helper.set_input_encoding(window, Encoding.SJIS); return true;},
  tty_enc_set_input_eucj : function(window)  { Helper.set_input_encoding(window, Encoding.EUCJP); return true;},
  tty_enc_set_input_utf8 : function(window)  { Helper.set_input_encoding(window, Encoding.UTF8); return true;},
  tty_enc_toggle_display_sjis : function(window)  { Helper.toggle_display_encoding(window, Encoding.SJIS); return true;},
  tty_enc_toggle_display_eucj : function(window)  { Helper.toggle_display_encoding(window, Encoding.EUCJP); return true;},
  tty_enc_toggle_display_utf8 : function(window)  { Helper.toggle_display_encoding(window, Encoding.UTF8); return true;},
  tty_toggle_scroll_key : function(window)  { Helper.toggle_priv_mode(window, Priv.ScrlTtyKey); return true;},
  tty_toggle_scroll_out : function(window)  { Helper.toggle_priv_mode(window, Priv.ScrlTtyOut); return true;},
  tty_toggle_bs_as_del : function(window)    { Helper.toggle_priv_mode(window, Priv.Backspace); return true;},
  tty_toggle_cjk_width : function(window)    { Helper.toggle_priv_mode(window, Priv.CjkWidth); return true;},
  wnd_menu : function(window)      { window.PopupMenu(false); return true;},
  wnd_sys_menu : function(window)      { window.PopupMenu(true); return true;},
  wnd_change_transp : function(window)    { window.Transp = (window.Transp+1)&3; return true;},
  wnd_toggle_scrollbar : function(window)    { window.Scrollbar = window.Scrollbar ^ 1; return true;},
  wnd_set_zorder_normal : function(window)  { window.ZOrder = WinZOrder.Normal; return true;},
  wnd_set_zorder_top : function(window)    { window.ZOrder = WinZOrder.Top;    return true;},
  wnd_set_zorder_bottom : function(window)  { window.ZOrder = WinZOrder.Bottom; return true;},
  wnd_inc_fontsize : function(window)    { window.FontSize+=2; return true;},
  wnd_dec_fontsize : function(window)    { window.FontSize-=2; return true;},
  app_toggle_console : function(window)    { app.ShowConsole = app.ShowConsole ? false : true; return true;}
};


var Events = new Object();
Events.base ={
  wnd_on_closed : function(window){
    WndList.remove(window);
    if(WndList.count() == 0){
      WndList.clear();
      PtyList.clear();
      app.Quit();
    }
  },
  wnd_on_key_down : function(window, key){
    for(var i=0; i < AccelKeys.length; i++){
      if(AccelKeys[i].match(key) && AccelKeys[i].exec(window))
        return;
    }
    var p = window.Pty;
    if(p) p.PutKeyboard(key);
  },
  wnd_on_mouse_down : function(window, button, x, y, modifier){
    var pty = window.Pty;
    if (pty) {
      var priv = pty.PrivMode;
        var mode = priv & Priv.MouseModeMask;
        if (mode === Priv.MouseMode_None) {
          return false;
        }
        var protocol = priv & Priv.MouseProtocolMask;
        var messsage;
        this._current_button = button;
        this._x = 0;
        this._y = 0;
        switch (protocol) {
          case Priv.MouseProtocol_X10:
          case Priv.MouseProtocol_X11:
              message = "\x1b[M" 
                      + String.fromCharCode(32 + button, 32 + x + 1, 32 + y + 1);
              pty.PutString(message);
              break;
          
          case Priv.MouseProtocol_Urxvt:
            message = "\x1b[" 
                    + (32 + button) + ";"
                    + (32 + x + 1) + ";"
                    + (32 + y + 1) 
                    + "M";
            pty.PutString(message);
            break;
          
          case Priv.MouseProtocol_Sgr:
            message = "\x1b[<" 
                    + button + ";"
                    + (x + 1) + ";"
                    + (y + 1) 
                    + "M";
            pty.PutString(message);
            break;
          
        default:
          break;
      }
      return true;
    }
    return false;
  },
  wnd_on_mouse_up : function(window, button, x, y, modifier){
    var pty = window.Pty;
    if (pty) {
      var priv = pty.PrivMode;
      var mode = priv & Priv.MouseModeMask;
      if (mode === Priv.MouseMode_None) {
        return false;
      }
      var protocol = priv & Priv.MouseProtocolMask;
      var messsage;
      this._current_button = 3;
      this._x = 0;
      this._y = 0;
      switch (protocol) {

        case Priv.MouseProtocol_X10:
        case Priv.MouseProtocol_X11:
          message = "\x1b[M" 
                  + String.fromCharCode(32 + 3, 32 + x + 1, 32 + y + 1);
          pty.PutString(message);
          break;
        
        case Priv.MouseProtocol_Urxvt:
          message = "\x1b[" 
                  + (32 + 3) + ";"
                  + (32 + x + 1) + ";"
                  + (32 + y + 1) 
                  + "M";
          pty.PutString(message);
          break;

        case Priv.MouseProtocol_Sgr:
          message = "\x1b[<" 
                  + button + ";"
                  + (x + 1) + ";"
                  + (y + 1) 
                  + "m";
          pty.PutString(message);
          break;

        default:
          break;
      }
      return true;
    }
    return false;
  },
  wnd_on_mouse_move : function(window, button, x, y, modifier){
     var pty = window.Pty;
    if (pty) {
      var priv = pty.PrivMode;
      var mode = priv & Priv.MouseModeMask;
      button = this._current_button;
      if (mode === Priv.MouseMode_None) {
        return false;
      }
      if (mode === Priv.MouseMode_Normal) {
        return true;
      }
      if (mode === Priv.MouseMode_ButtonEvent && button === 3) {
        return true;
      }
      var protocol = priv & Priv.MouseProtocolMask;
      var messsage;
      if (this._x === x && this._y === y){
        return true;
      }
      this._x = x;
      this._y = y;
      if (button != 3) {
        button |= 0x20;
      }
      switch (protocol) {

        case Priv.MouseProtocol_X10:
        case Priv.MouseProtocol_X11:
          message = "\x1b[M" 
                  + String.fromCharCode(32 + button, 32 + x + 1, 32 + y + 1);
          pty.PutString(message);
          break;
        
        case Priv.MouseProtocol_Urxvt:
          message = "\x1b[" 
                  + (32 + button) + ";"
                  + (32 + x + 1) + ";"
                  + (32 + y + 1) + "M";
          pty.PutString(message);
          break;

        case Priv.MouseProtocol_Sgr:
          message = "\x1b[<" 
                  + button + ";"
                  + (x + 1) + ";"
                  + (y + 1) + "M";
          pty.PutString(message);
          break;
        
        default: 
          return true;
      }
      return true;
    }
    return false;
  },
  wnd_on_mouse_wheel : function(window, x, y, key, delta){
     var pty = window.Pty;
    if (pty) {
      var priv = pty.PrivMode;
      var mode = priv & Priv.MouseModeMask;
      if (mode === Priv.MouseMode_None) {
         if(key & Keys.Ctrl){
          window.FontSize -= (delta * 2);
        }
        else{
          delta = (key & Keys.Alt) ? (delta*1) : (delta*5);
          if (priv & Priv.AplKP) {
            // for less(1)
            var key = (delta > 0) ? Keys.Up: Keys.Down;
            var count = Math.abs(delta);
            for (var i = 0; i < count; ++i) {
              pty.PutKeyboard(key);
            }
          } else {
            // for shell
            Helper.scroll_relative(window, 0, -delta);
          }
        }
      } else {
        var protocol = priv & Priv.MouseProtocolMask;
        var messsage;
        var button;
        if (delta > 0) {
          button = 64;
        } else {
          button = 65;
        }
        switch (protocol) {

          case Priv.MouseProtocol_X10:
          case Priv.MouseProtocol_X11:
            message = "\x1b[M" 
                    + String.fromCharCode(32 + button) 
                    + String.fromCharCode(32 + x + 1)
                    + String.fromCharCode(32 + y + 1);
            pty.PutString(message);
            break;
          
          case Priv.MouseProtocol_Urxvt:
            message = "\x1b[" 
                    + (32 + button) + ";"
                    + (32 + x + 1) + ";"
                    + (32 + y + 1) 
                    + "M";
            pty.PutString(message);
            break;

          case Priv.MouseProtocol_Sgr:
            message = "\x1b[<" 
                    + button + ";"
                    + (32 + x + 1) + ";"
                    + (32 + y + 1) 
                    + "M";
            pty.PutString(message);
            break;

          default:
            break;
        }
      }
    }
  },
  wnd_on_menu_init : function(window){
    var pty = window.Pty;
    {//category 0xA00
      var count=0;
      var nb = AccelKeys.length;
      for(var i=0; i < nb; i++){
        if(AccelKeys[i].menu){
          app.AddMenu(0xA00+i, AccelKeys[i].menu, false);
          count++;
        }
      }
      if(count>0){
        app.AddMenuSeparator();
      }
    }
    {//category 0xB00
      var nb = PtyList.count();
      for(var i=0; i<nb; i++){
        var p = PtyList.item(i);
        app.AddMenu(0xB00+i, (i+1)+" "+p.Title, (p==pty));
      }
      if(nb>0){
        app.AddMenuSeparator();
      }
    }
    {//category 0xC00
      var ienc = 0;
      var denc = 0;
      var priv = 0;
      var transp = window.Transp;
      var zorder = window.ZOrder;
      if(pty){
        ienc = pty.InputEncoding;
        denc = pty.DisplayEncoding;
        priv = pty.PrivMode;
      }
      app.AddMenu(0xC00+0,  "&Input ShiftJIS", ienc == Encoding.SJIS);
      app.AddMenu(0xC00+1,  "&Input EUC-JP",   ienc == Encoding.EUCJP);
      app.AddMenu(0xC00+2,  "&Input UTF8",     ienc == Encoding.UTF8);
      app.AddMenu(0xC00+3,  "&Display ShiftJIS", denc & Encoding.SJIS);
      app.AddMenu(0xC00+4,  "&Display EUC-JP",   denc & Encoding.EUCJP);
      app.AddMenu(0xC00+5,  "&Display UTF8",     denc & Encoding.UTF8);
      app.AddMenuSeparator();
      app.AddMenu(0xC00+20, "&Tty: Scroll Keypress", priv & Priv.ScrlTtyKey);
      app.AddMenu(0xC00+21, "&Tty: Scroll Output",   priv & Priv.ScrlTtyOut);
      app.AddMenu(0xC00+22, "&Tty: BS as DEL",       priv & Priv.Backspace);
      app.AddMenu(0xC00+23, "&Tty: CJK Width",       priv & Priv.CjkWidth);
      app.AddMenu(0xC00+24, "&Tty: Reset", false);
      app.AddMenuSeparator();
      if(app.IsDesktopComposition)
        app.AddMenu(0xC00+40, "&Win: Change Transp Mode", false);
      app.AddMenu(0xC00+41, "&Win: Scrollbar",     window.Scrollbar & 1);
      app.AddMenu(0xC00+42, "&Win: ZOrder Normal", zorder==WinZOrder.Normal);
      app.AddMenu(0xC00+43, "&Win: ZOrder Top",    zorder==WinZOrder.Top);
      app.AddMenu(0xC00+44, "&Win: ZOrder Bottom", zorder==WinZOrder.Bottom);
      app.AddMenu(0xC00+45, "&Win: FontSize+", false);
      app.AddMenu(0xC00+46, "&Win: FontSize-", false);
      app.AddMenuSeparator();
      app.AddMenu(0xC00+60, "Show Console", app.ShowConsole);
    }
  },
  wnd_on_menu_execute : function(window, id){
    var category = id & 0xF00;
    var index    = id & 0x0FF;
    if(category == 0xA00){
      AccelKeys[index].exec(window);
    }
    else if(category == 0xB00){
      window.Pty = PtyList.item(index);
    }
    else if(category == 0xC00){
      switch(index){
      case 0:  Commands.tty_enc_set_input_sjis(window);break;
      case 1:  Commands.tty_enc_set_input_eucj(window);break;
      case 2:  Commands.tty_enc_set_input_utf8(window);break;
      case 3:  Commands.tty_enc_toggle_display_sjis(window);break;
      case 4:  Commands.tty_enc_toggle_display_eucj(window);break;
      case 5:  Commands.tty_enc_toggle_display_utf8(window);break;
      case 20: Commands.tty_toggle_scroll_key(window);break;
      case 21: Commands.tty_toggle_scroll_out(window);break;
      case 22: Commands.tty_toggle_bs_as_del(window);break;
      case 23: Commands.tty_toggle_cjk_width(window);break;
      case 24: Commands.tty_reset(window);break;
      case 40: Commands.wnd_change_transp(window);break;
      case 41: Commands.wnd_toggle_scrollbar(window);break;
      case 42: Commands.wnd_set_zorder_normal(window);break;
      case 43: Commands.wnd_set_zorder_top(window);break;
      case 44: Commands.wnd_set_zorder_bottom(window);break;
      case 45: Commands.wnd_inc_fontsize(window);break;
      case 46: Commands.wnd_dec_fontsize(window);break;
      case 60: Commands.app_toggle_console(window);break;
      }
    }
  },
  wnd_on_title_init : function(window){
    var pty = window.Pty;
    var title = (pty) ? pty.Title : "(null)";
    var nbpty = PtyList.count();
    if(nbpty > 1){
      var i = 0;
      for(; i<nbpty; i++){
        if(PtyList.item(i) == pty)
          break;
      }
      title = "[" + (i+1) + "/" + (nbpty) + "] " + title;
    }
    window.Title = title;
  },
  wnd_on_drop : function(window, path, type, key){
    var pty = window.Pty;
    if(pty){
      if(type==1){//file path
        if(key & 0x02){//MK_RBUTTON
          path = app.ToCygwinPath(path);
        }
        path = Helper.escape_path(path) + " ";
      }
      else{//simple text
      }
      pty.PutString(path);
    }
  },
  pty_on_closed : function(pty){
    var nbwnd = WndList.count();
    for(var i=0; i < nbwnd; i++){
      var w = WndList.item(i);
      if(w.Pty == pty)
        w.Pty = null;
    }
    //
    PtyList.remove(pty);
    //
    var nbpty = PtyList.count();
    for(var i=nbwnd-1; i >= 0; i--){
      var w = WndList.item(i);
      if(w.Pty == null){
        if(nbwnd > 1 || nbpty < 1){
          w.Close();
          nbwnd--;
          continue;
        }
        w.Pty = PtyList.item(0);
      }
      Events.wnd_on_title_init(w);
    }
  },
  pty_on_update_screen : function(pty){
    var nbwnd = WndList.count();
    for(var i=0; i < nbwnd; i++){
      var w = WndList.item(i);
      if(w.Pty == pty)
        w.UpdateScreen();
    }
  },
  pty_on_update_title : function(pty){
    var nbwnd = WndList.count();
    for(var i=0; i < nbwnd; i++){
      var w = WndList.item(i);
      if(w.Pty == pty)
        Events.wnd_on_title_init(w);
    }
  },
  pty_on_req_font : function(pty, id){
    app.trace("pty_on_req_font " + id + "\n");
  },
  pty_on_req_move : function(pty, x, y){
    var nbwnd = WndList.count();
    for(var i=0; i < nbwnd; i++){
      var w = WndList.item(i);
      if(w.Pty == pty)
        w.Move(x,y);
    }
  },
  pty_on_req_resize : function(pty, x, y){
    var nbwnd = WndList.count();
    for(var i=0; i < nbwnd; i++){
      var w = WndList.item(i);
      if(w.Pty == pty)
        w.Resize(x,y);
    }
  },
  app_on_new_command : function(_args, _workdir){
    var opt = new ARGS(new VBArray(_args).toArray());
    var cfg = Config.clone();
    cfg.tty.workdir = _workdir;
    cfg.copyFrom_args(opt);

    var p = Helper.create_shell(cfg);

    //var act = Helper.get_active_window();
    //if(act != null){ act.Pty = p; return;}

    var w = Helper.create_window(cfg);
    w.Pty = p;
    w.Show();
  },
  app_on_startup : function(_args){
    {//env
      var ar = [];
      ar.push("/usr/local/bin");
      ar.push("/usr/bin");
      ar.push("/bin");
      ar.push("/usr/X11R6/bin");
      //uniq
      var old = app.Env("PATH").split(":");
      for(var i=0; i < old.length; i++){
        var k=0, op=old[i];
        for(; k < ar.length && ar[k] != op; k++);
        if(k >= ar.length) ar.push(op);
      }
      //set
      app.Env("PATH") = ar.join(":");
      app.Env("TERM") = "xterm";
      app.Env("COLORTERM") = "xterm";
      app.Env("CHERE_INVOKING") = "1";
    }

    //args
    var opt = new ARGS(new VBArray(_args).toArray());

    //user script
    var path = (opt.script != null) ? opt.script : HomeDir+"/.ck.config.js";
    app.trace("[file] \"" + path + "\"\n");
    app.EvalFile(path);

    //shortcut key
    AccelKeys.add(Config.accelkey.new_shell,  Commands.wnd_new_shell,    "&New Shell");
    AccelKeys.add(Config.accelkey.new_window,  Commands.wnd_new_window,  "&New Shell in New Window");
    AccelKeys.add(Config.accelkey.open_window,  Commands.wnd_open_window,  "&Open Window");
    AccelKeys.add(Config.accelkey.close_window,  Commands.wnd_close_window,  "Close &Window");
    AccelKeys.add(Config.accelkey.next_shell,  Commands.wnd_next_shell,  null);
    AccelKeys.add(Config.accelkey.prev_shell,  Commands.wnd_prev_shell,  null);
    AccelKeys.add(Config.accelkey.scroll_page_up,  Commands.tty_scroll_page_up,  null);
    AccelKeys.add(Config.accelkey.scroll_page_down,  Commands.tty_scroll_page_down,  null);
    AccelKeys.add(Config.accelkey.scroll_line_up,  Commands.tty_scroll_line_up,  null);
    AccelKeys.add(Config.accelkey.scroll_line_down,  Commands.tty_scroll_line_down,  null);
    AccelKeys.add(Config.accelkey.scroll_top,  Commands.tty_scroll_top,  null);
    AccelKeys.add(Config.accelkey.scroll_bottom,  Commands.tty_scroll_bottom,  null);
    AccelKeys.add(Config.accelkey.paste,    Commands.tty_paste,    "&Paste");
    AccelKeys.add(Config.accelkey.popup_menu,  Commands.wnd_menu,    null);
    AccelKeys.add(Config.accelkey.popup_sys_menu,  Commands.wnd_sys_menu,    null);

    //dup, set
    var cfg = Config.clone();
    cfg.copyFrom_args(opt);

    var p = Helper.create_shell(cfg);
    var w = Helper.create_window(cfg);
    w.Pty = p;
    w.Show();

    app.CurrentDirectory = HomeDir;
  }
};

Events.wnd_on_closed    = Events.base.wnd_on_closed;
Events.wnd_on_key_down    = Events.base.wnd_on_key_down;
Events.wnd_on_mouse_down  = Events.base.wnd_on_mouse_down;
Events.wnd_on_mouse_up    = Events.base.wnd_on_mouse_up;
Events.wnd_on_mouse_move  = Events.base.wnd_on_mouse_move;
Events.wnd_on_mouse_wheel  = Events.base.wnd_on_mouse_wheel;
Events.wnd_on_menu_init  = Events.base.wnd_on_menu_init;
Events.wnd_on_menu_execute  = Events.base.wnd_on_menu_execute;
Events.wnd_on_title_init  = Events.base.wnd_on_title_init;
Events.wnd_on_drop    = Events.base.wnd_on_drop;
Events.pty_on_closed    = Events.base.pty_on_closed;
Events.pty_on_update_screen  = Events.base.pty_on_update_screen;
Events.pty_on_update_title  = Events.base.pty_on_update_title;
Events.pty_on_req_font    = Events.base.pty_on_req_font;
Events.pty_on_req_move    = Events.base.pty_on_req_move;
Events.pty_on_req_resize  = Events.base.pty_on_req_resize;
Events.app_on_new_command  = Events.base.app_on_new_command;
Events.app_on_startup    = Events.base.app_on_startup;

//EOF
