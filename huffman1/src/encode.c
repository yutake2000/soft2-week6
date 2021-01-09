#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "encode.h"


// 各シンボルの数を数える為にポインタを定義
static const int nsymbols = 256;
//int symbol_count[nsymbols];
int * symbol_count;

static int *codes[nsymbols];
static int code_len[nsymbols];

// ノードを表す構造体
typedef struct node
{
  int symbol;
  int count;
  struct node *left;
  struct node *right;
} Node;

// このソースで有効なstatic関数のプロトタイプ宣言
// 一方で、ヘッダファイルは外部からの参照を許す関数の宣言のみ


// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(FILE *fp);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数（根となる構造体へのポインタを返す）
static Node *build_tree(void);

// 木を深さ優先で操作する関数
static void traverse_tree(const int depth, const Node *np);

// コード語を表示する関数
static void print_code(const int symbol);



// 以下 static関数の実装
static void count_symbols(FILE *fp)
{

  symbol_count = (int*)calloc(nsymbols, sizeof(int));
  
  int c;
  while((c = fgetc(fp)) != EOF) {
    symbol_count[c]++;
  }

}

static Node *create_node(int symbol, int count, Node *left, Node *right)
{
  Node *ret = (Node *)malloc(sizeof(Node));
  *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right};
  return ret;
}

static Node *pop_min(int *n, Node *nodep[])
{
  // Find the node with the smallest count
  // カウントが最小のノードを見つけてくる
  int argmin = 0;
  for (int i = 0; i < *n; i++) {
    if (nodep[i]->count < nodep[argmin]->count) {
      argmin = i;
    }
  }

  Node *node_min = nodep[argmin];

  // Remove the node pointer from nodep[]
  // 見つかったノード以降の配列を前につめていく
  for (int i = argmin; i < (*n) - 1; i++) {
    nodep[i] = nodep[i + 1];
  }
  // 合計ノード数を一つ減らす
  (*n)--;

  return node_min;
}

static Node *build_tree()
{
  int n = 0;
  Node *nodep[nsymbols];

  for (int i = 0; i < nsymbols; i++) {
    // カウントの存在しなかったシンボルには何もしない
    if (symbol_count[i] == 0) continue;

    // 以下は nodep[n++] = create_node(i, symbol_count[i], NULL, NULL); と1行でもよい
    nodep[n] = create_node(i, symbol_count[i], NULL, NULL);
    n++;
  }

  const int dummy = -1; // ダミー用のsymbol を用意しておく
  while (n >= 2) {
    Node *node1 = pop_min(&n, nodep);
    Node *node2 = pop_min(&n, nodep);

    nodep[n++] = create_node(dummy, node1->count + node2->count, node1, node2);

  }
  // 気にした後は symbol_counts は free
  free(symbol_count);
  return (n==0)?NULL:nodep[0];
}

static void print_code(const int symbol) {

  int last_one = -1;
  for (int i=0; i<code_len[symbol]; i++) {
    if (codes[symbol][i] == 1)
      last_one = i;
  }

  for (int i=0; i<last_one; i++) {
    if (codes[symbol][i] == 0)
      printf("│ ");
    else
      printf("  ");
  }

  if (last_one == -1) {
    printf("├─");
    last_one = 0;
  } else {
    printf("└─");
  }

  for (int i=last_one+1; i<code_len[symbol]; i++) {
    printf("┬─");
  }

  printf(" ");

  for (int i=0; i<code_len[symbol]; i++) {
    printf("%d", codes[symbol][i]);
  }

  if (symbol == '\n')
    printf(" '\\n' \n");
  else if (symbol == '\r')
    printf(" '\\r' \n");
  else
    printf(" '%c' \n", symbol);

}

// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
static void traverse_tree(const int depth, const Node *np)
{

  static int code[nsymbols] = {0};

  if (np->symbol >= 0) {
    int *temp = (int*)calloc(depth, sizeof(int));
    memcpy(temp, code, sizeof(int) * depth);
    codes[np->symbol] = temp;
    code_len[np->symbol] = depth;
    print_code(np->symbol);
  }
  
  if (np->left != NULL) {
    code[depth] = 0;
    traverse_tree(depth + 1, np->left);

    code[depth] = 1;
    traverse_tree(depth + 1, np->right);

    code[depth] = 0;
  }

}

static void write_symbol(FILE *fout, const int symbol) {

  static char buffer = 0;
  static int buffer_len = 0;

  if (symbol == -1) {
    buffer <<= 8 - buffer_len;
    fwrite(&buffer, sizeof(char), 1, fout);
    buffer = 0;
    buffer_len = 0;
    return;
  }

  for (int i=0; i<code_len[symbol]; i++) {
    buffer <<= 1;
    buffer |= codes[symbol][i];
    buffer_len++;
    if (buffer_len == 8) {
      fwrite(&buffer, sizeof(char), 1, fout);
      buffer = 0;
      buffer_len = 0;
    }
  }

}

void compress(FILE *fin, FILE *fout) {

  int c;
  int sum_len = 0;
  while((c = fgetc(fin)) != EOF) {
    sum_len += code_len[c];
  }
  fwrite(&sum_len, sizeof(int), 1, fout);

  rewind(fin);

  for (int i=0; i<nsymbols; i++) {
    fwrite(&code_len[i], sizeof(int), 1, fout);
  }

  for (int i=0; i<nsymbols; i++) {
    if (code_len[i] > 0) {
      write_symbol(fout, i);
    }
  }
  
  while((c = fgetc(fin)) != EOF) {
    write_symbol(fout, c);
  }
  write_symbol(fout, -1);

}

// この関数のみ外部 (main) で使用される (staticがついていない)
int encode(FILE *fin)
{
  count_symbols(fin);
  Node *root = build_tree();

  if (root == NULL){
    fprintf(stderr,"A tree has not been constructed.\n");
    return EXIT_FAILURE;
  }

  for (int i=0; i<nsymbols; i++) {
    codes[i] = NULL;
    code_len[i] = 0;
  }
  
  printf("┐\n");
  traverse_tree(0, root);

  return EXIT_SUCCESS;
}
