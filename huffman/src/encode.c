#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "encode.h"


// 各シンボルの数を数える為にポインタを定義
// 数を数える為に、1byteの上限+1で設定しておく
static const int nsymbols = 256 + 1; 
//int symbol_count[nsymbols];
int * symbol_count;

// ノードを表す構造体
typedef struct node
{
  int symbol;
  int count;
  struct node *left;
  struct node *right;
  int code_len;
  int *code;
} Node;

// このソースで有効なstatic関数のプロトタイプ宣言
// 一方で、ヘッダファイルは外部からの参照を許す関数の宣言のみ


// ファイルを読み込み、static配列の値を更新する関数
static void count_symbols(const char *filename);

// 与えられた引数でNode構造体を作成し、そのアドレスを返す関数
static Node *create_node(int symbol, int count, Node *left, Node *right);

// Node構造体へのポインタが並んだ配列から、最小カウントを持つ構造体をポップしてくる関数
// n は 配列の実効的な長さを格納する変数を指している（popするたびに更新される）
static Node *pop_min(int *n, Node *nodep[]);

// ハフマン木を構成する関数（根となる構造体へのポインタを返す）
static Node *build_tree(void);

// 木を深さ優先で操作する関数
static void traverse_tree(const int depth, int *code, int *code_len, Node *np);

// コード語を表示する関数
static void print_code(const Node *np);



// 以下 static関数の実装
static void count_symbols(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "error: cannot open %s\n", filename);
    exit(1);
  }

  symbol_count = (int*)calloc(nsymbols, sizeof(int));
  
  int c;
  while((c = fgetc(fp)) != EOF) {
    symbol_count[c]++;
  }

  fclose(fp);
}

static Node *create_node(int symbol, int count, Node *left, Node *right)
{
  Node *ret = (Node *)malloc(sizeof(Node));
  *ret = (Node){ .symbol = symbol, .count = count, .left = left, .right = right, .code_len = 0};
  ret->code = (int*)calloc(260, sizeof(int));
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

static void print_code(const Node *np) {

  printf("%c ", np->symbol);
  for (int i=0; i<np->code_len; i++) {
    printf("%d", np->code[i]);
  }
  printf("\n");

}

// Perform depth-first traversal of the tree
// 深さ優先で木を走査する
static void traverse_tree(const int depth, int *code, int *code_len, Node *np)
{

  if (np->symbol >= 0) {
    memcpy(np->code, code, sizeof(int) * *code_len);
    np->code_len = *code_len;
    print_code(np);
  }
  
  if (np->left != NULL) {
    code[(*code_len)++] = 0;
    traverse_tree(depth + 1, code, code_len, np->left);

    code[(*code_len)++] = 1;
    traverse_tree(depth + 1, code, code_len, np->right);

    code[*code_len+1] = 0;
  }

  (*code_len)--;
}

// この関数のみ外部 (main) で使用される (staticがついていない)
int encode(const char *filename)
{
  count_symbols(filename);
  Node *root = build_tree();

  if (root == NULL){
    fprintf(stderr,"A tree has not been constructed.\n");
    return EXIT_FAILURE;
  }
  
  int* code = (int*)calloc(260, sizeof(int));
  int code_len = 0;
  traverse_tree(0, code, &code_len, root);
  return EXIT_SUCCESS;
}
