
CC = cl.exe
LD = link.exe
#CC = icl.exe /Qvc9
#LD = xilink.exe

#RM = rm -f
RM = del

CFLAGS  = /nologo /MT /W3 /Ox /fp:fast /arch:SSE2 /EHsc /GF /GR- /GS- /GL
LDFLAGS = /nologo /release /machine:X86 /opt:ref /LTCG
#CFLAGS  = /nologo /MTd /W3 /Wp64 /Od /fp:fast /arch:SSE2 /EHsc /GF /GR- /GS- /Zi /DDEBUG /D_DEBUG
#LDFLAGS = /nologo /debug /machine:X86

APP_CFLAGS =
#APP_CFLAGS = /Qprof-gen /Qprof-dir D:\\cygwin\\tmp\\icl-pgo
#APP_CFLAGS = /Qprof-use /Qprof-dir D:\\cygwin\\tmp\\icl-pgo



TARGET = ck.exe ck.app.dll  #ck.con.exe

OBJS   = config.obj app.res app.obj window.obj util.obj \
	pty.obj screen.obj encoding.obj encoding_table.obj

CLEANFILES = \
	config.pch config.pchi \
	tlbimp.obj interface.tlb interface.tlh interface.h interface_i.c interface_p.c dlldata.c \
	main_launch.obj main_launch.res \
	main_con.obj main_con.res \
	ck.con.exe main_con_gcc.res.o \
	vc100.idb vc100.pdb ck.ilk ck.pdb \
	ck.con.ilk ck.con.pdb ck.con.exp ck.con.lib ck.con.exe.stackdump \
	ck.app.ilk ck.app.pdb ck.app.exp ck.app.lib \
	cygwin1_con.exp cygwin1_con.lib \
	cygwin1_app.exp cygwin1_app.lib

.SUFFIXES: .cpp .obj  .def .lib  .idl .tlb

all: $(TARGET)

ck.app.dll: cygwin1_app.lib interface.tlh $(OBJS)
	$(LD) /dll /base:0x1E000000 $(LDFLAGS) /out:$@ $(OBJS)

ck.con.exe: cygwin1_con.lib main_con.res main_con.obj
	$(LD) $(LDFLAGS) /out:$@ main_con.obj chkstk.obj main_con.res

ck.exe: main_launch.res main_launch.obj
	$(LD) $(LDFLAGS) /out:$@ main_launch.obj main_launch.res

main_launch.res: rsrc.rc ck.ico
	RC.Exe /l 0x409 /d TARGET=0 /fo $@ rsrc.rc
main_con.res: rsrc.rc ck.ico
	RC.Exe /l 0x409 /d TARGET=1 /fo $@ rsrc.rc
app.res: rsrc.rc ck.ico interface.tlb script_sys.js
	RC.Exe /l 0x409 /d TARGET=2 /fo $@ rsrc.rc


interface.tlh: interface.tlb
	cl.exe /nologo /c tlbimp.cpp
main_launch.obj: main_launch.cpp
	$(CC) $(CFLAGS) /c main_launch.cpp
main_con.obj: main_con.cpp
	$(CC) $(CFLAGS) /c main_con.cpp
encoding_table.obj: encoding_table.cpp encoding.h
	$(CC) $(CFLAGS) $(APP_CFLAGS) /c encoding_table.cpp
config.obj: config.cpp config.h
	$(CC) $(CFLAGS) $(APP_CFLAGS) /Ycconfig.h /c config.cpp
.cpp.obj:
	$(CC) $(CFLAGS) $(APP_CFLAGS) /Yuconfig.h /c $<
.def.lib:
	lib.exe /nologo /machine:X86 /out:$@ /def:$<
.idl.tlb:
	Midl.Exe /nologo $<

clean: cle
	$(RM) $(TARGET)
cle:
	$(RM) $(OBJS) $(CLEANFILES) *~


encoding.obj: encoding.cpp config.h interface.tlh encoding.h
screen.obj  : screen.cpp   config.h interface.tlh screen.h
pty.obj     : pty.cpp      config.h interface.tlh util.h encoding.h screen.h
utilobj     : util.cpp     config.h interface.tlh util.h
window.obj  : window.cpp   config.h interface.tlh util.h
app.obj     : app.cpp      config.h interface.tlh util.h
