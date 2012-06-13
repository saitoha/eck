

= eck (embedded ck) について

eckは、Kazuo Ishii氏作のcygwin ck terminal emulator(http://www.geocities.jp/meir000/ck/、以下ck)をもとにフォークしたプロジェクトです。
従来のckにかなり大幅な改造を加え、他のアプリケーションへ組み込んで使用可能にしようという試みです。
ライセンスはGPLv3です。
注意：まだはじめたばかりです。

== 端末エミュレーションの精度、スペックを向上させる。
== 拡張マウスレポーティングやOSC52 base64 Paste、Bracketed paste modeなど、流行の機能を取り入れる。
== Ck::PtyとCk::Appをより分離された形する。イベント通知機構はコネクションポイントプロトコルを使い、両者を互いに祖結合な状態に置く。
== Ck::Ptyのクラスファクトリを書き、アウトプロセスサーバーとして公開する。cygwin1.dllを読み込む以上インプロセスサーバーは厳しいので。

#########################################################################
# コンパイル

  Visual C++ 2010 Express Edition と cygwin gcc-4が必要です。

  VS2010コマンドプロンプト

  > StartMenu
    > All Programs
      > Microsoft Visual Studio 2010 Express
        > Visual Studio Command Prompt (2010)

  を開いて

  cd /d C:\download\ck-3.3.4\src [enter]
  nmake.exe [enter]

  これでコンパイルできます。
  次に ck.con.exeだけ cygwin gccで作り直します。
  cygwinから

  cd /cygdrive/c/download/ck-3.3.4/src [enter]
  windres -o main_con_gcc.res.o -D TARGET=1 -i rsrc.rc [enter]
  gcc-4 -o ck.con.exe -pipe -s -O2 -Wall -static-libgcc main_con_gcc.c main_con_gcc.res.o [enter]

  出来上がった ck.exe ck.con.exe ck.app.dll を /bin にコピーして入れ、
  ck.exeをダブルクリックして起動すれば完成。

#########################################################################
# その他

  文字コード変換テーブル(encoding_table.cpp)は .NET frameworkのデータからC#で生成しています。
    csc.exe gen_enc.cs [enter]
    gen_enc.exe [enter]
