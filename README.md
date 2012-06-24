
eck (embedded ck) について
==========================

eckは、Kazuo Ishii氏作の[cygwin ck terminal emulator](http://www.geocities.jp/meir000/ck/)(以下ck)をもとにフォークしたプロジェクトです。
従来のckにかなり大幅な改造を加え、他のアプリケーションへ組み込んで使用可能にしようというプロジェクトです。
eckをバックエンドにしたアプリケーションとしてのck派生版も同時に配布していくつもりです。

ソースコードは以下で配布しています。
https://github.com/saitoha/eck

ライセンスはGPLv3です。
注意：まだはじめたばかりです。

以下のような作業を予定しています。
* 端末エミュレーションの精度、スペックを向上させる。
* 拡張マウスレポーティングやOSC52 base64 Pasteなどの、流行の機能を取り入れる。

現在、本家とくらべて、以下の機能が追加されています。

* 拡張マウスレポーティング（urxvt/SGR）への対応
* Bracketed Paste Modeへの対応

#########################################################################
# コンパイル

  Visual C++ 2010 Express Edition と cygwin gcc-4が必要です。

  コンパイルはcygwinプロンプト上で行います。
  
  make

  make install

  /eck

#########################################################################
# その他

  文字コード変換テーブル(encoding_table.cpp)は .NET frameworkのデータからC#で生成しています。

    csc.exe gen_enc.cs [enter]

    gen_enc.exe [enter]
