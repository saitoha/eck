
CL=cl.exe
GCC=gcc
LD=link.exe
VSDIR=C:\Program Files\Microsoft Visual Studio 10.0\VC
MSSDK=C:\Program Files\Microsoft SDKs\Windows\v7.0A
CP=cp -f
PROGRAM=eck
ZIP=zip

#CC = icl.exe /Qvc9
#LD = xilink.exe

#RM = rm -f
RM=rm -f

CFLAGS  = /nologo \
		  /MT \
		  /W3 \
		  /Ox \
		  /fp:fast \
		  /arch:SSE2 \
		  /EHsc \
		  /GF \
		  /GR- \
		  /GS- \
		  /GL \
		  /I"$(VSDIR)\include" \
		  /I"$(MSSDK)\Include" 
LDFLAGS = /nologo \
		  /release \
		  /machine:X86 \
		  /opt:ref \
		  /LTCG \
		  /LIBPATH:"$(VSDIR)\Lib" \
		  /LIBPATH:"$(MSSDK)\Lib"
#CFLAGS  = /nologo /MTd /W3 /Wp64 /Od /fp:fast /arch:SSE2 /EHsc /GF /GR- /GS- /Zi /DDEBUG /D_DEBUG
#LDFLAGS = /nologo /debug /machine:X86

APP_CFLAGS =
#APP_CFLAGS = /Qprof-gen /Qprof-dir D:\\cygwin\\tmp\\icl-pgo
#APP_CFLAGS = /Qprof-use /Qprof-dir D:\\cygwin\\tmp\\icl-pgo


VCBIN :=/cygdrive/c/Program Files/Microsoft Visual Studio 10.0/VC/bin
SDKBIN :=/cygdrive/c/Program Files/Microsoft SDKs/Windows/v7.0A/bin
IDEPATH :=/cygdrive/c/Program Files/Microsoft Visual Studio 10.0/Common7/IDE
PATH :=$(VCBIN):$(SDKBIN):$(IDEPATH):$(PATH)

TARGET = $(PROGRAM).exe $(PROGRAM).app.dll $(PROGRAM).con.exe

OBJS   = config.obj app.res app.obj window.obj util.obj \
	pty.obj screen.obj encoding.obj encoding_table.obj

CLEANFILES = \
	config.pch config.pchi \
	tlbimp.obj interface.tlb interface.tlh interface.h interface_i.c interface_p.c dlldata.c \
	main_launch.obj main_launch.res \
	main_con.obj main_con.res \
	$(PROGRAM).con.exe main_con_gcc.res.o \
	vc100.idb vc100.pdb $(PROGRAM).ilk $(PROGRAM).pdb \
	$(PROGRAM).con.ilk $(PROGRAM).con.pdb $(PROGRAM).con.exp $(PROGRAM).con.lib $(PROGRAM).con.exe.stackdump \
	$(PROGRAM).app.ilk $(PROGRAM).app.pdb $(PROGRAM).app.exp $(PROGRAM).app.lib \
	cygwin1_con.exp cygwin1_con.lib \
	cygwin1_app.exp

.SUFFIXES: .cpp .obj  .def .lib  .idl .tlb

all:
	export LANG=C && $(MAKE) $(TARGET) 2>&1 | iconv -f sjis -t UTF-8

$(PROGRAM).app.dll: interface.tlh $(OBJS) cygwin1_app.lib
	$(LD) /dll /base:0x1E000000 $(LDFLAGS) /out:$@ $(OBJS) cygwin1_app.lib
#$(PROGRAM).con.exe: cygwin1_con.lib main_con.res main_con.obj
#	$(LD) $(LDFLAGS) /out:$@ main_con.obj chkstk.obj main_con.res
$(PROGRAM).con.exe:
	$(GCC) main_con_gcc.c -o $@

$(PROGRAM).exe: main_launch.res main_launch.obj
	$(LD) $(LDFLAGS) /out:$@ main_launch.obj main_launch.res

main_launch.res: rsrc.rc ck.ico
	RC.Exe /nologo /l 0x409 /d TARGET=0 /fo $@ rsrc.rc
main_con.res: rsrc.rc ck.ico
	RC.Exe /nologo /l 0x409 /d TARGET=1 /fo $@ rsrc.rc
app.res: rsrc.rc ck.ico interface.tlb script_sys.js
	RC.Exe /nologo /l 0x409 /d TARGET=2 /fo $@ rsrc.rc


interface.tlh: interface.tlb
	$(CL) $(CFLAGS) /nologo /c tlbimp.cpp
main_launch.obj: main_launch.cpp
	$(CL) $(CFLAGS) /c main_launch.cpp
main_con.obj: main_con.cpp
	$(CL) $(CFLAGS) /c main_con.cpp
encoding_table.obj: encoding_table.cpp encoding.h
	$(CL) $(CFLAGS) $(APP_CFLAGS) /c encoding_table.cpp
config.obj: config.cpp config.h
	$(CL) $(CFLAGS) $(APP_CFLAGS) /Ycconfig.h /c config.cpp
.cpp.obj:
	$(CL) $(CFLAGS) $(APP_CFLAGS) /Yuconfig.h /c $<
.def.lib:
	lib.exe /nologo /machine:X86 /out:$@ /def:$<

interface.tlb: interface.idl
	midl.exe /nologo /client none /I"$(MSSDK)\Include" $<

zip: $(TARGET)
	$(CP) ../README.md ../COPYING.txt .
	$(ZIP) $(PROGRAM)-$$(date +%y%m%d%H%M%S)-bin.zip $(TARGET) README.md COPYING.txt
	$(RM) README.md COPYING.txt

clean:
	$(RM) $(OBJS) $(CLEANFILES) *~
	$(RM) $(TARGET)

install: $(TARGET)
	$(CP) $^ /usr/bin

uninstall: $(TARGET)
	$(CP) $^ /usr/bin

run: all
	./$(PROGRAM).con.exe

debug: all
	ntsd.exe ./$(PROGRAM).con.exe


encoding.obj: encoding.cpp config.h interface.tlh encoding.h
screen.obj  : screen.cpp   config.h interface.tlh screen.h
pty.obj     : pty.cpp      config.h interface.tlh util.h encoding.h screen.h
utilobj     : util.cpp     config.h interface.tlh util.h
window.obj  : window.cpp   config.h interface.tlh util.h
app.obj     : app.cpp      config.h interface.tlh util.h

