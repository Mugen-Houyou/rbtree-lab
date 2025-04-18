#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>


void rotateLeft(rbtree* t, node_t** axis){
  node_t* y;

  y = (*axis)->right;
  (*axis)->right = y->left;

  if (y->left != t->nil)
    y->left->parent = *axis;

  y->parent = (*axis)->parent;

  if ((*axis)->parent == t->nil)
    t->root = y;
  else if ((*axis) == (*axis)->parent->left)
    (*axis)->parent->left = y;  
  else 
    (*axis)->parent->right = y;
  
  y->left = (*axis);
  (*axis)->parent = y;
}

void rotateRight(rbtree* t, node_t** axis){
  node_t* y;

  y = (*axis)->left;
  (*axis)->left = y->right;

  if (y->right != t->nil)
    y->right->parent = *axis;

  y->parent = (*axis)->parent;

  if ((*axis)->parent == t->nil)
    t->root = y;
  else if ((*axis) == (*axis)->parent->left)
    (*axis)->parent->left = y;  
  else 
    (*axis)->parent->right = y;
  
  y->right = (*axis);
  (*axis)->parent = y;
}

void removeAllNodes(rbtree* t, node_t** node){ 
  // 주의: 이건 노드들만. RBT 자체와 t->nil은 밖에서 free()할 것.
	if (*node == NULL || *node == t->nil) return;

  removeAllNodes(t, &((*node)->left));
  removeAllNodes(t, &((*node)->right));
  free(*node);
  *node = NULL;
}

node_t *newNode(rbtree* t, const key_t key){
  node_t* newChild;

  newChild = (node_t*) calloc(1, sizeof(node_t));
  if (newChild == NULL)
    return NULL;

  newChild->key = key;
  newChild->left = t->nil;
  newChild->right = t->nil;
  newChild->parent = t->nil;
  newChild->color = (t->root == t->nil) ? RBTREE_BLACK : RBTREE_RED;
  
  return newChild;
}

void restoreRbtOrder(rbtree *t, node_t *z) {
  int infLoop;
  node_t *me, *uncle, *grandp; // 문장 너무 길어짐

  me = z;
  
  for (;;){
    // 부모가 없으면 돌아가 (필요없으니까)
    if (me->parent == NULL || me->parent == t->nil)
      break;

    // 부모가 블랙이면 돌아가 (필요없으니까)
    if (me->parent->color == RBTREE_BLACK)
      break;

    // parent가 루트였다!
    if (me->parent == t->root) { 
      me->parent->color = RBTREE_BLACK;
      break;
    }
  
    // 조부모가 없으면 부모가 루트니까 블랙으로 칠하고 끝
    if (me->parent->parent == t->nil){
      me->parent->color = RBTREE_BLACK;
      break;
    }

    grandp = me->parent->parent;
    uncle = (me->parent == grandp->left) ? grandp->right : grandp->left;

    // 케이스 1.1. 부모, 삼촌이 모두 RED - 좌측 버전
    if (me->parent->color == RBTREE_RED && uncle->color == RBTREE_RED) {      
      me->parent->color = RBTREE_BLACK;
      uncle->color = RBTREE_BLACK;
      grandp->color = RBTREE_RED;
      me = grandp; // 조부모로 올라감 - 이유는 R-R을 R-B로 바꿨으니 굳이 부모 볼 이유가 없음
      continue; // 이걸로 삼촌은 무.적.권. BLACK.
    }

    // 케이스 2.1., 3.1. 부모는 RED, 숙부는 BLACK, 삼각형 형태 - 좌측 버전
    // 케이스 2.2., 3.2. 부모는 RED, 숙부는 BLACK, 삼각형 형태 - 우측 버전
    if (me->parent == grandp->left) { // Left‑Right 삼각형 => Left‑Left 일자
      if (me == me->parent->right) {
        me = me->parent;
        rotateLeft(t, &me);
      }
      me->parent->color = RBTREE_BLACK;
      grandp->color = RBTREE_RED;
      rotateRight(t, &grandp);
      break;
    } else { // Right‑Left 삼각형 => Right‑Right 일자
      if (me == me->parent->left) {
        me = me->parent;
        rotateRight(t, &me);
      }
      me->parent->color = RBTREE_BLACK;
      grandp->color = RBTREE_RED;
      rotateLeft(t, &grandp);
      break;
    }
  }

  // 루트를 흑색으로
  t->root->color = RBTREE_BLACK;
}

rbtree *new_rbtree(void) {
  rbtree *root;

  printf("now in rbtree;");
  // root 생성
  root = calloc(1, sizeof(rbtree));
  if (root == NULL)
    return NULL;
  
  // root의 nil 생성, 색칠
  root->nil = calloc(1, sizeof(node_t));
  if (root->nil == NULL) {
    free(root);
    return NULL;
  }
  root->nil->color = RBTREE_BLACK;

  // 자기자신, 부모, 자식 전부 다 nil로.
  root->root = root->nil;
  root->root->parent = root->nil;
  root->root->left = root->nil;
  root->root->right = root->nil;

  return root;
}

/*
  "구현하는 ADT가 multiset이므로 이미 같은 key의 값이 존재해도 하나 더 추가합니다."
  즉 중복 허용.
*/
node_t *rbtree_insert(rbtree *t, const key_t key) {
  node_t *child, *parent, *insertPos, *parentPrev;
  
  parent = t->nil;   // 부모 후보
  insertPos = t->root;  // 탐색용
  // parent_prev = parent;

  // 넣을 위치 찾기
  while (insertPos != t->nil) {
    parent = insertPos;
    
    if (key < insertPos->key)
      insertPos = insertPos->left;
    else
      insertPos = insertPos->right; // 같은 키면 일단 오른쪽 ㄱㄱ
    
    // parent_prev = insert_pos;
  }

  child = newNode(t, key); // 넣을 자식 노드 준비

  // 넣기
  if (parent == t->nil)
    t->root = child; // 루트가 없음
  else if (child->key < parent->key)
    parent->left = child; // 왼쪽에
  else
    parent->right = child; // 오른쪽에

  child->parent = parent;

  restoreRbtOrder(t, child);

  return child;
}


void delete_rbtree(rbtree *t) {
  if (t==NULL) 
    return;   
  removeAllNodes(t, &(t->root));
  free(t->nil);
  free(t);
  t->root = NULL;
  t->nil = NULL;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  node_t *currNode;
  
  currNode = t->root;  // 루트부터

  // 탐색
  while (currNode != t->nil) {
    if (key < currNode->key)
      currNode = currNode->left;
    else if (key > currNode->key)
      currNode = currNode->right;
    else if (key == currNode->key)
      return currNode;
    else break;
  }
  
  return NULL;
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  return t->root;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // TODO: implement erase
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}
