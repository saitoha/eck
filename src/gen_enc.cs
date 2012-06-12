using System;
using System.IO;
using System.Text;
using System.Collections.Generic;

struct Data{
	public int	sta;
	public int	end;
};

static class App{
	static void PutTable(TextWriter outputC, string name, bool a2u, ushort[] ar){
		for(int i=0 ; i <= 0xff; i++){
			if(ar[i] == 0)
				ar[i] = (ushort)i;
		}

		Data[]  fd = new Data[0x100];
		for(int i=0 ; i <= 0xff; i++){
			int hi = (i<<8);
			int sta = -1;
			int end = -1;
			for(int k=0; k<=0xff; k++){
				if(ar[hi|k] != 0){
					if(sta==-1) sta=k;
					end=k;
				}
			}
			fd[i].sta = sta;
			fd[i].end = end;
			if(end==-1){
				continue;
			}

			outputC.WriteLine("static const {0} _{1}_{2:X4}_data []={{ //{3:X4} {4:X4}", (a2u)? "WCHAR": "WORD", name, hi, hi|sta, hi|end);
			if((sta & 15) != 0){
				outputC.Write("\t");
				for(int k=0; k < (sta & 15); k++)
					outputC.Write("       ");
			}
			for(int k=sta; k<=end; k++){
				if((k & 15) == 0 ) outputC.Write("\t");
				outputC.Write("0x{0:X4},", ar[hi|k]);
				if((k & 15) == 15) outputC.WriteLine("");
			}
			if((end & 15) != 15) outputC.WriteLine("");
			outputC.WriteLine("};");
		}
		//
		outputC.WriteLine("extern const struct _encoding_table_{0} _{1}_table []={{", (a2u)? "a": "u", name);
		for(int i=0 ; i <= 0xff; i++){
			int hi = (i<<8);
			if(fd[i].end == -1)
				outputC.WriteLine("\t{0,0xffff,0},");
			else
				outputC.WriteLine("\t{{ 0x{0:X4},0x{1:X4},_{2}_{3:X4}_data }},", hi|fd[i].end, hi|fd[i].sta, name, hi);
		}
		outputC.WriteLine("};");
	}

	static void EncDec(TextWriter outputC, int codepage){
		Decoder dec = Encoding.GetEncoding(codepage).GetDecoder();
		Encoder enc = Encoding.GetEncoding(codepage).GetEncoder();
		dec.Fallback = new DecoderReplacementFallback(new string(new char[1]{'\x01'}));
		enc.Fallback = new EncoderReplacementFallback(new string(new char[1]{'\x01'}));

		byte[] bytes = new byte[3];
		char[] chars = new char[3];
		int n;
		ushort us;
		ushort[] bin = new ushort[0x10000];

		for(int i = 0; i <= 0xffff; i++){
			bin[i]=0;
			if(i < 0x100){
				bytes[0] = (byte)(i);
				n = dec.GetChars(bytes, 0, 1, chars, 0, true);
			}
			else{
				bytes[0] = (byte)(i>>8);
				bytes[1] = (byte)(i);
				n = dec.GetChars(bytes, 0, 2, chars, 0, true);
			}
			if(n == 1 && chars[0] != '\x01'){
				us = (ushort) chars[0];
				bin[i] = us;
			}
		}
		PutTable(outputC, (codepage==932) ? "sjis_to_ucs" : "eucj_to_ucs", true, bin);

		for(uint i = 0; i <= 0xffff; i++){
			bin[i]=0;
			chars[0] = (char) i;
			n = enc.GetBytes(chars,0,1, bytes,0,true);
			if(n==1)
				us = (ushort) (bytes[0]);
			else if(n==2)
				us = (ushort) ((bytes[0]<<8) | bytes[1]);
			else
				continue;
			if(us == 0x01)
				continue;
			bin[i] = us;
		}
		PutTable(outputC, (codepage==932) ? "ucs_to_sjis" : "ucs_to_eucj", false, bin);
	}

	static void Main(string[] args){
		TextWriter outputC = new StreamWriter("encoding_table.cpp", false, Encoding.UTF8);

		outputC.WriteLine("#define _WIN32_WINNT 0x600");
		outputC.WriteLine("#define WIN32_LEAN_AND_MEAN 1");
		outputC.WriteLine("#define UNICODE  1");
		outputC.WriteLine("#define _UNICODE 1");
		outputC.WriteLine("#include <Windows.h>");
		outputC.WriteLine("enum Encoding;");
		outputC.WriteLine("#include \"encoding.h\"");
		outputC.WriteLine("namespace Ck{");
		outputC.WriteLine("namespace Enc{");

		EncDec(outputC, 932);
		EncDec(outputC, 51932);

		outputC.WriteLine("");
		outputC.WriteLine("}//namespace Enc");
		outputC.WriteLine("}//namespace Ck");
		outputC.WriteLine("//EOF");

		outputC.Close();
	}
}
//EOF
