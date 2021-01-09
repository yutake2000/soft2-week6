# ファイルの圧縮・解凍

## 機能
複数ファイルの圧縮を実装しました。

## 使い方
```
bin/huffman compress <output filename> <input filenames...>
bin/huffman expand <input filename>
```

### 例
```
make
bin/huffman compress compressed.zipdat sample.txt helloworld.txt news.txt
bin/huffman expand compressed.zipdat
```

## ファイル構造
-	圧縮したファイル数(int)
-	それぞれのファイルに対して
	-	ファイル名の長さ(int)
	-	ファイル名(char[])
	-	符号列本体の長さ(int)
	-	0から255までの記号に対して
		-	それぞれの符号語のビット列の長さ(int)
		-	それぞれの符号語(可変ビット数)
	-	符号列(可変ビット数)