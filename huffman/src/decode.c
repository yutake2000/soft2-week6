#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "decode.h"

static const int nsymbols = 256;
static int *codes[nsymbols];
static int code_len[nsymbols] = {0};

int min(int a, int b) {
  return (a < b ? a : b);
}

// 指定したビット数だけ読み込む
static void read_symbol(int *arr, int len, FILE *fin) {

  static unsigned char buffer;
  static int buffer_i = 8;

  for (int i=0; i<len; i++) {
    if (buffer_i == 8) {
      int nitems = fread(&buffer, sizeof(char), 1, fin);
      if (nitems == 0) {
        arr[i] = -1;
      } else {
        buffer_i = 0;
      }
    }
    if (buffer_i < 8) {
      arr[i] = (buffer >> (7-buffer_i)) & 1;
      buffer_i++;
    }
  }

}

// 前方一致する符号を見つけ、その記号をファイルに書き込む(接頭語条件を満たしているため)
// 符号のビット数を返す
static int write_symbol(int *buffer, int buffer_len, int buffer_i, FILE *fout) {

  for (int symbol=0; symbol<nsymbols; symbol++) {
    if (code_len[symbol] == 0) continue;

    int same = 1;
    for (int j=0; j<code_len[symbol]; j++) {
      if (buffer_i + j >= buffer_len || buffer[buffer_i + j] != codes[symbol][j]) {
        same = 0;
        break;
      }
    }

    if (same) {
      fwrite(&symbol, sizeof(char), 1, fout);
      return code_len[symbol];
    }

  }

  return 0;

}

void expand(FILE *fin, FILE *fout) {

  int file_len; // 本体のビット数
  fread(&file_len, sizeof(int), 1, fin);

  // 0-255に対応する符号語の長さ(ビット数)を読み込む
  for (int i=0; i<nsymbols; i++) {
    fread(&code_len[i], sizeof(int), 1, fin);
  }

  // 0-255に対応する符号語を読み込む
  for (int i=0; i<nsymbols; i++) {
    if (code_len[i] > 0) {
      int *temp = (int*)calloc(code_len[i], sizeof(int));
      read_symbol(temp, code_len[i], fin);
      codes[i] = temp;
      for (int j=0; j<code_len[i]; j++) {
        printf("%d ", codes[i][j]);
      }
      printf("\n");
    }
  }

  int buffer_len = 256;
  int buffer_i = 256;
  int buffer[buffer_len];
  while(file_len > 0) {
    // 前に詰めて空いた分だけ読み込む
    if (file_len < buffer_i) {
      read_symbol(buffer + (buffer_len - buffer_i), file_len, fin);
      for (int i = buffer_len - buffer_i + file_len; i < buffer_len; i++) {
        buffer[i] = -1;
      }
      file_len = 0;
    } else {
      read_symbol(buffer + (buffer_len - buffer_i), buffer_i, fin);
      file_len -= buffer_i;
    }

    buffer_i = 0;
    while (buffer_i < buffer_len) {
      int bits = write_symbol(buffer, buffer_len, buffer_i, fout);
      buffer_i += bits;
      if (bits == 0) // 符号語の途中でバッファの終端に達した場合
        break; 
    }

    // 読み込んだ分前に詰める
    for (int i=0; i<buffer_len - buffer_i; i++) {
      buffer[i] = buffer[i + buffer_i];
    }
  }

}